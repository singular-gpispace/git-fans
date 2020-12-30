#include "gitfanInterface.hpp"

#include <util-gitfan/simplexfaceiterator.hpp>
#include <util-gitfan/conversion.hpp>
#include <util-gitfan/paths.hpp>
#include <util-gitfan/singular_library_wrapper.hpp>
#include <util-gitfan/filecontent_operations.hpp>

#include <storage/conestorage.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/join.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/functional/hash.hpp>

#include <pnetc/type/file/op.hpp>

#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <iomanip>
#include <algorithm>
#include <memory>

namespace gitfan {
namespace interface
{
  namespace fs=boost::filesystem;

  namespace
  {
    storage::IConeStorage& storage(const std::string& options)
    {
      static auto s = storage::IConeStorage::build(options);
      return *s;
    }
  }

  NO_NAME_MANGLING
  storage::Options initStorageOptions
    (const std::vector<std::string>& options,
     const boost::filesystem::path& workingDirectory)
  {
    return storage::Options(options, Paths::storageDirectory(workingDirectory));
  }

  NO_NAME_MANGLING
  void processPrecomputedData
    (const pnet_problem_data& data,
     const pnet_directory& workingDirectory,
     pnet_list& orbitCones,
     pnet_file& symmetryGroupOnHashesFile)
  {
    const fs::path orbitConesFile = data.precomputed_orbit_cones.path;
    const fs::path sourceDirectory = orbitConesFile.parent_path();
    const fs::path targetDirectory =
      Paths::orbitConesDirectory(workingDirectory.path);
    fs::create_directories(targetDirectory);
    for (const std::string& file : fhg::util::read_lines(orbitConesFile))
    {
      const fs::path source = sourceDirectory / file;
      const fs::path target = targetDirectory / file;
      fs::copy_file(source, target);
      orbitCones.push_back(
        pnetc::type::file::to_value(pnet_file(target.string())));
    }

    const fs::path groupTarget =
      Paths::symmetryGroupOnHashesFile(workingDirectory.path);
    if (data.precomputed_symmetry_group_on_hashes.valid)
    {
      fs::create_directories(groupTarget.parent_path());
      fs::copy_file( data.precomputed_symmetry_group_on_hashes.path
                   , groupTarget
                   );
    }
    else
    {
      singular::gitfan::writeTrivialGroupActionToSSIFile
        ( orbitCones.size()
        , groupTarget
        );
    }
    symmetryGroupOnHashesFile.path = groupTarget.string();
  }

  NO_NAME_MANGLING
  pnet_file removeOrbitStructureAndReturnSymmetryGroupOnHashesFile
    (const pnet_problem_data& data,
     const pnet_directory& workingDirectory,
     pnet_list& orbitInfos)
  {
    singular::gitfan::GroupAction groupActionOnFlattened
      (data.symmetry_group_size);
    pnet_list flattened;
    fs::create_directories(Paths::orbitConesDirectory(workingDirectory.path));
    for ( const auto& orbitData : orbitInfos
        | boost::adaptors::transformed(&as<pnet_map>)
        )
    {
      pnet_list orbit = as<pnet_list>(orbitData.at(Keys::ELEMENTS()));
      const fs::path groupActionOnOrbitFile =
        pnetc::type::file::from_value(orbitData.at(Keys::GROUP_ACTION())).path;
      const auto groupActionOnOrbit =
        singular::gitfan::readGroupAction(groupActionOnOrbitFile);
      const unsigned int shift = flattened.size();

      for ( unsigned int permutationIndex = 0
          ; permutationIndex < data.symmetry_group_size
          ; permutationIndex++
          )
      {
        for (const auto& value : groupActionOnOrbit[permutationIndex])
        {
          groupActionOnFlattened[permutationIndex].push_back(value + shift);
        }
      }

      for (const auto& orbitCone : orbit)
      {
        const fs::path orbitConeIntermediatePath =
          pnetc::type::file::from_value(orbitCone).path;
        const fs::path orbitConeFinalPath =
          Paths::orbitConesDirectory(workingDirectory.path)
          / orbitConeIntermediatePath.filename();
        fs::copy_file(orbitConeIntermediatePath, orbitConeFinalPath);
        flattened.push_back(pnetc::type::file::to_value(
          pnet_file(orbitConeFinalPath.string())));
      }
    }

    orbitInfos = flattened;
    fs::path target = Paths::symmetryGroupOnHashesFile(workingDirectory.path);
    singular::gitfan::writeGroupActionToSSIFile(groupActionOnFlattened, target);
    return pnet_file(target.string());
  }

  NO_NAME_MANGLING
  void preprocessProblemData
    (pnet_problem_data& data,
     const pnet_directory& workingDirectory)
  {
    singular::gitfan::readAndSetDimensionParameters(data);

    fs::path movingConePath(data.moving_cone.path);
    if (data.moving_cone.valid && !fs::exists(movingConePath))
    {
      singular::gitfan::computeMovingConeAndWriteSSIFile(data, movingConePath);
    }

    if (data.symmetry_group.valid && !data.simplex_orbit_representatives.valid)
    {
      const auto path =
        fs::path(workingDirectory.path) / "simplex_orbit_representatives";
      pnet_list orbits = singular::gitfan::findOrbitRepresentatives(data);

      if (!path.parent_path().empty())
      {
        fs::create_directories(path.parent_path());
      }
      fs::ofstream o(path);
      for (const pnet_list& orbit : orbits
          | boost::adaptors::transformed (&as<pnet_list>)
          )
      {
        std::list<int> orbitAsInts;
        std::transform(orbit.begin(), orbit.end(),
                       std::back_inserter(orbitAsInts), &as<int>);
        o << fhg::util::join(orbitAsInts, ",") << std::endl;
      }
      o.close();

      data.simplex_orbit_representatives.valid = true;
      data.simplex_orbit_representatives.path = path.string();
    }
  }

  NO_NAME_MANGLING
  pnet_list_range createSimplexFaces
    (int r,
     int k,
     pnet_list_range& last,
     unsigned long range,
     bool& finished)
  {
    SimplexFaceIterator iter;

    if (last.start.empty())
    {
      iter = SimplexFaceIterator(r, k);
    }
    else
    {
      std::vector<int> vector = conversion::unwrap<int>(last.start);
      iter = SimplexFaceIterator(r, vector);
      iter += last.length;
    }

    pnet_list_range result;
    result.start = conversion::wrap(*iter);
    result.length = iter.increment(range);
    finished = iter.isEnd();
    last = result;
    return result;
  }

  NO_NAME_MANGLING
  pnet_list_range createSimplexFacesFromFile
    (const pnet_problem_data& data,
     pnet_list_range& last,
     unsigned long range,
     bool& finished)
  {
    /* We save the start position of the package in pnet_list_range.start[0]
     * and the (exclusive) end position in pnet_list_range.start[1].
     * pnet_list_range.length determines the amount of lines in between.
     */
    pnet_list_range result;
    boost::filesystem::ifstream ifs
      ( fs::path(data.simplex_orbit_representatives.path)
      , boost::filesystem::ifstream::in | boost::filesystem::ifstream::binary
      );
    if (!last.start.empty())
    {
      result.start.push_back(last.start.back());
      ifs.seekg(as<unsigned long>(last.start.back()));
    }
    else
    {
      result.start.push_back(0ul);
    }
    std::string line;
    unsigned long readLines = 0;
    while (readLines < range && std::getline(ifs, line))
    {
      readLines++;
    }
    result.start.push_back((unsigned long)ifs.tellg());
    result.length = readLines;
    finished = ifs.eof();
    last = result;
    return result;
  }

  NO_NAME_MANGLING
  pnet_list computeFullDimAFacesFromRange
    (const pnet_problem_data& data,
     const pnet_list_range& simplexFaces)
  {
    singular::init();
    std::vector<int> start = conversion::unwrap<int>(simplexFaces.start);
    SimplexFaceIterator iter(data.r, start);
    lists lFaces = static_cast<lists>(omAllocBin (slists_bin));
    lFaces->Init(simplexFaces.length);
    for (unsigned int i = 0; i < simplexFaces.length; i++)
    {
      intvec* ivFace = conversion::createIntvec(*iter);
      lFaces->m[i].rtyp = INTVEC_CMD;
      lFaces->m[i].data = static_cast<void*>(ivFace);
      ++iter;
    }
    return singular::gitfan::computeFullDimAFaces(data, lFaces);
  }

  NO_NAME_MANGLING
  pnet_list computeFullDimAFacesFromFile
    (const pnet_problem_data& data,
     const pnet_list_range& simplexFaces)
  {
    pnet_list facesList = conversion::loadListPartially
      ( fs::path(data.simplex_orbit_representatives.path)
      , as<unsigned long>(simplexFaces.start.front())
      , simplexFaces.length
      );
    lists lFaces = conversion::toSingularList(facesList, true);
    return singular::gitfan::computeFullDimAFaces(data, lFaces);
  }

  NO_NAME_MANGLING
  pnet_file computeOrbitCone
    (const pnet_list& aface,
     const pnet_problem_data& data,
     const pnet_directory& workingDirectory)
  {
    fs::path path(Paths::intermediateOrbitInfoDirectory(workingDirectory.path)
      / conversion::listToHexString(aface, data.r));
    singular::gitfan::computeOrbitConeAndWriteSSIFile(data, aface, path);
    return pnet_file(path.string());
  }

  NO_NAME_MANGLING
  pnet_list intersectWithMovingCone
    (const pnet_problem_data& data,
     const pnet_list& orbitCones)
  {
    return singular::gitfan::
      intersectWithMovingConeAndOverwriteSSIFiles(data, orbitCones);
  }

  NO_NAME_MANGLING
  bool isLastOfAllEqualOrbitConeOrbits
    (const pnet_problem_data& data,
     const pnet_list& orbitInfos,
     pnet_list::const_iterator orbitInfo)
  {
    filecontent::EqualTo equals(std::make_shared<filecontent::Cache>(2));
    const pnet_list& elements =
      as<pnet_list>(as<pnet_map>(*orbitInfo).at(Keys::ELEMENTS()));
    // We assume that the unique orbit cone per orbit with the lexicographically
    // minimal file content is located at the front of the list
    // (guaranteed by expandOrbitConeOrbit)
    const fs::path minimalConePath =
      pnetc::type::file::from_value(elements.front()).path;
    ++orbitInfo;
    return std::all_of(orbitInfo, orbitInfos.cend(),
      [&](const pnet_value& other) -> bool
      {
        const pnet_list& otherElements =
          as<pnet_list>(as<pnet_map>(other).at(Keys::ELEMENTS()));
        const fs::path otherMinimalConePath =
          pnetc::type::file::from_value(otherElements.front()).path;
        return !equals(minimalConePath, otherMinimalConePath);
      });
  }

  NO_NAME_MANGLING
  pnet_map expandOrbitConeOrbit
    (const pnet_problem_data& data,
     const pnet_directory& workingDirectory,
     const pnet_file& representative,
     unsigned long orbitNumber)
  {
    pnet_map result;
    result.insert({Keys::ELEMENTS(), pnet_list()});

    /* Load the group action in order to to obtain the index h of the
     * permutation resulting from composing permutations with index i and j.
     * As we have to lookup the index of the composed permutation, we speed
     * this process up by creating a hashtable beforehand.
     */
    const auto groupAction = singular::gitfan::readSymmetryGroupAction(data);
    std::unordered_map< singular::gitfan::Permutation
                      , unsigned int
                      , typename boost::hash<singular::gitfan::Permutation>
                      > groupActionIndices(2 * data.symmetry_group_size);
    for (unsigned int i = 0; i < groupAction.size(); i++)
    {
      groupActionIndices.insert({ groupAction[i], i });
    }

    const fs::path targetDirectory =
      Paths::intermediateOrbitInfoDirectory(workingDirectory.path);
    const std::vector<fs::path> orbitWithDuplicates =
      singular::gitfan::computeOrbitConeOrbitWithDuplicatesAndWriteSSIFiles
        ( data
        , representative
        , targetDirectory / ("oc_" + std::to_string(orbitNumber))
        );

    const auto cache = std::make_shared<filecontent::Cache>(5);
    filecontent::Compare compare(cache);
    std::unordered_map< fs::path
                      , unsigned int
                      , filecontent::Hash
                      , filecontent::EqualTo >
      elementIndices( 2 * data.symmetry_group_size
                    , filecontent::Hash(cache)
                    , filecontent::EqualTo(cache)
                    );
    std::vector<unsigned int> elementIndexToOrbitWithDuplicatesIndex;
    std::vector<unsigned int> orbitWithDuplicatesIndexToElementIndex
      (orbitWithDuplicates.size());
    fs::path minimalElement = orbitWithDuplicates.front();
    pnet_list& elements = boost::get<pnet_list>(result[Keys::ELEMENTS()]);
    for (unsigned int i = 0 ; i < orbitWithDuplicates.size(); i++)
    {
      const auto& element = orbitWithDuplicates[i];
      const auto insertionResult =
        elementIndices.insert({element, elements.size()});
      orbitWithDuplicatesIndexToElementIndex[i] = insertionResult.first->second;
      if (insertionResult.second)
      {
        elements.push_back
          (pnetc::type::file::to_value(pnet_file(element.string())));
        elementIndexToOrbitWithDuplicatesIndex.push_back(i);
        if (compare(element, minimalElement))
        {
          minimalElement = element;
        }
      }
    }

    // Guarantee that the orbit cone with the lexicographically
    // minimal file content is located at the front of the list
    // (isLastOfAllEqualOrbitConeOrbits depends on it)
    const fs::path& firstElement = orbitWithDuplicates[0];
    unsigned int minimalElementIndex = elementIndices[minimalElement];
    auto minimalElementPos = std::next(elements.begin(), minimalElementIndex);
    {
      const auto tmp = elements.front();
      elements.front() = *minimalElementPos;
      *minimalElementPos = tmp;
    }
    elementIndices[firstElement] = minimalElementIndex;
    elementIndices[minimalElement] = 0;
    {
      unsigned int tmp = elementIndexToOrbitWithDuplicatesIndex[0];
      elementIndexToOrbitWithDuplicatesIndex[0] =
        elementIndexToOrbitWithDuplicatesIndex[minimalElementIndex];
      elementIndexToOrbitWithDuplicatesIndex[minimalElementIndex] = tmp;
    }
    std::transform( orbitWithDuplicatesIndexToElementIndex.begin()
                  , orbitWithDuplicatesIndexToElementIndex.end()
                  , orbitWithDuplicatesIndexToElementIndex.begin()
                  , [=] (unsigned int i)
                    {
                      return i == 0 ? minimalElementIndex
                                    : (i == minimalElementIndex ? 0 : i);
                    }
                  );

    /* Compute the group action on the orbit. Note that since gitfan.lib and
     * thus (for compatibility) gitfanparallel.lib implement the group action
     * on orbit cones by taking the preimage cone under A_sigma and not the
     * image cone, we have to consider the opposite group of the symmetry group
     * acting on the cones. For this reason
     * "groupAction[dupIndex] * groupAction[permutationIndex]"
     * is in right order, although counterintuitive when checking the
     * computations by hand.
     */
    singular::gitfan::GroupAction groupActionOnOrbit(groupAction.size());
    for ( unsigned int permutationIndex = 0
        ; permutationIndex < groupAction.size()
        ; permutationIndex++
        )
    {
      auto& permutationOnOrbit = groupActionOnOrbit[permutationIndex];
      permutationOnOrbit.reserve(elements.size());
      for (unsigned int i = 0; i < elements.size(); i++)
      {
        unsigned int dupIndex = elementIndexToOrbitWithDuplicatesIndex[i];
        const auto& translation =
          groupAction[dupIndex] * groupAction[permutationIndex];
        unsigned int imageDupIndex = groupActionIndices[translation];
        permutationOnOrbit.push_back
          (orbitWithDuplicatesIndexToElementIndex[imageDupIndex]);
      }
    }

    const fs::path groupActionTarget =
      Paths::intermediateOrbitInfoDirectory(workingDirectory.path)
      / ("group_action_on_orbit_" + std::to_string(orbitNumber));
    singular::gitfan::writeGroupActionToSSIFile( groupActionOnOrbit
                                               , groupActionTarget
                                               );
    result.insert(
      { Keys::GROUP_ACTION()
      , pnetc::type::file::to_value(pnet_file(groupActionTarget.string()))
      });

    return result;
  }

  NO_NAME_MANGLING
  void removeIntermediateOrbitInfo(const pnet_directory& workingDirectory)
  {
    fs::remove_all
      (Paths::intermediateOrbitInfoDirectory(workingDirectory.path));
  }

  NO_NAME_MANGLING
  boost::optional<pnet_list> computeInitialCone
    (const pnet_problem_data& data,
     const pnet_list& orbitCones,
     const pnet_file& symmetryGroupSSIFile)
  {
    if (orbitCones.empty())
    {
      return boost::none;
    }
    return boost::optional<pnet_list>(::singular::gitfan::computeInitialCone
    ( data
    , orbitCones
    , symmetryGroupSSIFile.path)
    );
  }

  NO_NAME_MANGLING
  pnet_list getExpandableCones
    (const std::string& storageOptions,
     const unsigned long& jobSize)
  {
    pnet_list result;
    for (std::string& hexedHash : storage(storageOptions).unmarkCones(jobSize))
    {
      result.push_back(conversion::hexStringToList(hexedHash));
    }
    return result;
  }

  NO_NAME_MANGLING
  pnet_list_of_sets expandCones
    (const pnet_problem_data& data,
     const pnet_list& orbitCones,
     const pnet_file& symmetryGroupSSIFile,
     const pnet_list& conesToExpand)
  {
    return singular::gitfan::computeNeighbourHashes
      ( data
      , orbitCones
      , symmetryGroupSSIFile.path
      , conesToExpand
      );
  }

  NO_NAME_MANGLING
  unsigned long insertNeighbours
    (const std::string& storageOptions,
     const unsigned long& numberOfOrbitCones,
     const pnet_list_of_sets& neighbours,
     const pnet_list& expandedCones)
  {
    if (neighbours.empty())
    {
      return 0ul;
    }

    unsigned long insertionCount = 0;
    auto& store = storage(storageOptions);
    pnet_set neighbour_set;
    pnet_list expandedCone;
    BOOST_FOREACH (boost::tie(neighbour_set, expandedCone),
                   boost::combine(neighbours, expandedCones
                     | boost::adaptors::transformed (&as<pnet_list>))
                  )
    {
      for (const pnet_list& neighbour : neighbour_set
          | boost::adaptors::transformed (&as<pnet_list>)
          )
      {
        std::string hexedHash =
          conversion::listToHexString(neighbour, numberOfOrbitCones);
        if (store.addMarkedCone(hexedHash))
        {
          insertionCount++;
        }
      }
      std::string expandedHexedHash =
        conversion::listToHexString(expandedCone, numberOfOrbitCones);
      store.endOfTransaction(expandedHexedHash);
    }
    return insertionCount;
  }

  NO_NAME_MANGLING
  void outputOrbitConesOrder
    (const pnet_directory& workingDirectory,
     const pnet_list& orbitCones)
  {
    fs::path outputPath
      (Paths::orbitConeOrderFile(fs::path(workingDirectory.path)));
    if (!outputPath.parent_path().empty())
    {
      fs::create_directories(outputPath.parent_path());
    }
    fs::ofstream o(outputPath);
    for (const auto& orbitCone : orbitCones
        | boost::adaptors::transformed(&pnetc::type::file::from_value)
        )
    {
      o << fs::path(orbitCone.path).filename().string() << std::endl;
    }
    o.close();
  }

  NO_NAME_MANGLING
  void outputGitCones
    (const pnet_directory& workingDirectory,
     const std::string& storageOptions)
  {
    bool success = storage(storageOptions).saveConesPersistently
      (Paths::gitConesFile(workingDirectory.path));

    if (!success)
    {
      throw std::runtime_error
        ("Failed to save computed git cone hashes to disk.");
    }
  }

  NO_NAME_MANGLING
  void outputLoadGitConesSingularFile
    (const pnet_directory& workingDirectory,
     const pnet_list& orbitCones)
  {
    const fs::path outputFile =
      Paths::loadGitConesSingularFile(workingDirectory.path);
    const fs::path gcFile =
      Paths::gitConesFile(workingDirectory.path);
    const fs::path symmetryFile =
      Paths::symmetryGroupOnHashesFile(workingDirectory.path);
    const fs::path singularLib =
      Paths::loadGitConesSingularLibrary(workingDirectory.path);

    const fs::path relGcFile =
      fs::path(gcFile).lexically_relative(outputFile.parent_path());
    const fs::path relSymmetryFile =
      fs::path(symmetryFile).lexically_relative(outputFile.parent_path());
    const fs::path relSingularLib =
      fs::path(singularLib).lexically_relative(outputFile.parent_path());
    const fs::path relOcDirectory =
      Paths::orbitConesDirectory(workingDirectory.path)
        .lexically_relative(outputFile.parent_path());

    fs::copy_file(config::loadGitConesLibrary(), singularLib);

    fs::ofstream o(outputFile);
    o << std::left
      << "/////////////////////////////////////////////////////////////" << "\n"
      << "// Loads the computed git cones into the singular          //" << "\n"
      << "// environment.                                            //" << "\n"
      << "// Output: var ORBIT_CONES; list(cone),                    //" << "\n"
      << "//         var GIT_CONES; list(cone).                      //" << "\n"
      << "// Dependencies: " << std::setw(41)
                             << relSymmetryFile.string()
                             << " //" << "\n"
      << "//               " << std::setw(41)
                             << (relOcDirectory.string() + " (directory)")
                             << " //" << "\n"
      << "//               " << std::setw(41)
                             << relSingularLib.string()
                             << " //" << "\n"
      << "/////////////////////////////////////////////////////////////" << "\n"
      << "LIB \"" << relSingularLib.string() << "\";" << "\n";

    o << "\n// Load all orbit cones" << "\n"
      << "list ORBIT_CONES;" << "\n";
    for (const auto& orbitCone : orbitCones
        | boost::adaptors::transformed(&pnetc::type::file::from_value)
        | boost::adaptors::indexed(1)
        )
    {
      o << "ORBIT_CONES[" << orbitCone.index() << "] = read(\"ssi:r "
        << fs::path(orbitCone.value().path)
            .lexically_relative(outputFile.parent_path()).string()
        << "\");" << "\n";
    }

    o << "\n// Define git cone hash representatives" << "\n"
      << "list GIT_CONES;" << "\n";
    const std::vector<std::string> hexHashes = fhg::util::read_lines(gcFile);
    for (const auto& hexHash : hexHashes
        | boost::adaptors::indexed(1)
        )
    {
      const auto hash = conversion::hexStringToList(hexHash.value());
      std::list<int> hashAsInts;
      std::transform(hash.begin(), hash.end(),
                     std::back_inserter(hashAsInts), &as<int>);
      o << "GIT_CONES[" << hexHash.index() << "] = intvec("
        << fhg::util::join(hashAsInts, ",") << ");\n";
    }

    o << "\n// Compute the hashes of all orbits wrt. the symmetry group action"
         << "\n"
      << "// Comment out if git cone representatives are sufficient" << "\n"
      << "def groupAction = read(\"ssi:r " << relSymmetryFile.string()
         << "\");" << "\n"
      << "GIT_CONES = computeOrbits(GIT_CONES, groupAction);" << "\n"
      << "kill groupAction;" << "\n";

    o << "\n// Convert all hashes to git cones" << "\n"
      << "GIT_CONES = hashesToGITCones(GIT_CONES, ORBIT_CONES);" << "\n";
  }
}}
