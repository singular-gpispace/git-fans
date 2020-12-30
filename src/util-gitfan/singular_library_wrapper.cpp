#include <util-gitfan/singular_library_wrapper.hpp>

#include <util-gitfan/singular_commands.hpp>
#include <util-gitfan/conversion.hpp>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <pnetc/type/file/op.hpp>

#include <config.hpp>

namespace singular {
namespace gitfan
{
  namespace fs=boost::filesystem;
  using namespace ::gitfan::conversion;
  using namespace ::gitfan;

  static std::string const& SUFFIX()
  {
    static std::string const _("___");
    return _;
  }

  std::string const& Symbols::IDEAL()
  {
    static std::string const _("I" + SUFFIX());
    return _;
  }

  std::string const& Symbols::TORUS_ACTION()
  {
    static std::string const _("Q" + SUFFIX());
    return _;
  }

  std::string const& Symbols::ORBIT_CONES()
  {
    static std::string const _("OC" + SUFFIX());
    return _;
  }

  std::string const& Symbols::MOVING_CONE()
  {
    static std::string const _("MC" + SUFFIX());
    return _;
  }

  std::string const& Symbols::SYMMETRY_GROUP()
  {
    static std::string const _("SG" + SUFFIX());
    return _;
  }

  std::string const& Symbols::SYMMETRY_GROUP_ON_CONES()
  {
    static std::string const _("SG_CONES" + SUFFIX());
    return _;
  }

  std::string const& Symbols::SYMMETRY_GROUP_ON_HASHES()
  {
    static std::string const _("SG_HASHES" + SUFFIX());
    return _;
  }

  // Singular does not create parent directories on its own.
  void createParent(const fs::path& path)
  {
    if (!path.parent_path().empty())
    {
      fs::create_directories(path.parent_path());
    }
  }

  void killGlobalSymbols_TESTING_ONLY()
  {
    init();
    if (symbol_exists(Symbols::IDEAL()))
      { kill(Symbols::IDEAL().c_str()); }
    if (symbol_exists(Symbols::TORUS_ACTION()))
      { kill(Symbols::TORUS_ACTION().c_str()); }
    if (symbol_exists(Symbols::ORBIT_CONES()))
      { kill(Symbols::ORBIT_CONES().c_str()); }
    if (symbol_exists(Symbols::MOVING_CONE()))
      { kill(Symbols::MOVING_CONE().c_str()); }
    if (symbol_exists(Symbols::SYMMETRY_GROUP()))
      { kill(Symbols::SYMMETRY_GROUP().c_str()); }
    if (symbol_exists(Symbols::SYMMETRY_GROUP_ON_CONES()))
      { kill(Symbols::SYMMETRY_GROUP_ON_CONES().c_str()); }
    if (symbol_exists(Symbols::SYMMETRY_GROUP_ON_HASHES()))
      { kill(Symbols::SYMMETRY_GROUP_ON_HASHES().c_str()); }
  }

  GroupAction readGroupActionFromSymbol(const std::string& symbol)
  {
    int size = std::stoi(get_result("size(" + symbol + ");"));
    GroupAction group(size);

    call_and_discard("intvec img;");
    for (int i = 0; i < size; i++)
    {
      call_and_discard("img = " + symbol + "[" + std::to_string(i + 1)
        + "].image;");
      conversion::insert(getIntvec("img"),std::back_inserter(group[i]));
      // Decrement values since singular indices are 1-based.
      for (unsigned int& value : group[i])
      {
        value--;
      }
    }
    kill("img");
    return group;
  }

  GroupAction readSymmetryGroupAction
    (const pnet_problem_data& data)
  {
    loadSymmetryGroup(data);
    return readGroupActionFromSymbol(Symbols::SYMMETRY_GROUP());
  }

  GroupAction readGroupAction
    ( const boost::filesystem::path& ssiFile )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi("g", ssiFile.string());
    GroupAction result = readGroupActionFromSymbol("g");
    kill("g");
    return result;
  }

  void writeGroupActionToSSIFile
    ( const GroupAction& action
    , const boost::filesystem::path& target
    )
  {
    // Increment values since singular indices are 1-based.
    std::vector<std::vector<int> > incAction(action.size());
    for (unsigned int i = 0; i < action.size(); i++)
    {
      incAction[i].resize(action[i].size());
      for (unsigned int j = 0; j < action[i].size(); j++)
      {
        incAction[i][j] = static_cast<int>(action[i][j] + 1);
      }
    }
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    call_and_discard("list g;");
    for (unsigned int i = 0; i < incAction.size(); i++)
    {
      put("iv", conversion::createIntvec(incAction[i]));
      call_and_discard("g[" + std::to_string(i + 1)
        + "] = permutationFromIntvec(iv);");
      kill("iv");
    }
    createParent(target);
    write_ssi("g", target.string());
    kill("g");
  }

  void readAndSetDimensionParameters(pnet_problem_data& data)
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi_if_not_defined (Symbols::TORUS_ACTION(), data.Q.path);
    std::string rows = get_result("nrows(" + Symbols::TORUS_ACTION() + ");");
    std::string cols = get_result("ncols(" + Symbols::TORUS_ACTION() + ");");
    data.k = std::stoi(rows);
    data.r = std::stoi(cols);
    loadSymmetryGroup(data);
    std::string size = get_result("size(" + Symbols::SYMMETRY_GROUP() + ");");
    data.symmetry_group_size = std::stoi(size);
  }

  void loadMovingCone(const pnet_problem_data& data)
  {
    init();
    if (symbol_exists(Symbols::MOVING_CONE()))
    {
      return;
    }

    if (!data.moving_cone.valid)
    {
      load_library(config::gitfanInterpreterLibrary().string());
      load_ssi_if_not_defined(Symbols::TORUS_ACTION(), data.Q.path);
      call_and_discard("cone " + Symbols::MOVING_CONE()
        + " = coneViaPoints(transpose(" + Symbols::TORUS_ACTION() + "));");
    }
    else
    {
      load_ssi_if_not_defined(Symbols::MOVING_CONE(), data.moving_cone.path);
    }
  }

  void loadSymmetryGroup(const pnet_problem_data& data)
  {
    init();
    if (symbol_exists(Symbols::SYMMETRY_GROUP()))
    {
      return;
    }
    load_library(config::gitfanInterpreterLibrary().string());
    if (data.symmetry_group.valid)
    {
      load_ssi_if_not_defined(Symbols::SYMMETRY_GROUP(),
                              data.symmetry_group.path);
    }
    else
    {
      if (data.r <= 0)
      {
        std::logic_error("Trying to load the symmetry group before computing "
                           "the problem dimension.");
      }
      call_and_discard("def " + Symbols::SYMMETRY_GROUP()
        + " = trivialGroupAction(" + std::to_string(data.r) + ");");
    }
  }

  void loadSymmetryGroupOnCones(const ::gitfan::pnet_problem_data& data)
  {
    init();
    if (symbol_exists(Symbols::SYMMETRY_GROUP_ON_CONES()))
    {
      return;
    }
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi_if_not_defined(Symbols::TORUS_ACTION(), data.Q.path);
    loadSymmetryGroup(data);
    call_and_discard("def " + Symbols::SYMMETRY_GROUP_ON_CONES() +
      " = groupActionOnQImage(" + Symbols::SYMMETRY_GROUP() + "," +
      Symbols::TORUS_ACTION() + ");");
  }

  void computeMovingConeAndWriteSSIFile
    ( const pnet_problem_data& data
    , const boost::filesystem::path& file
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi_if_not_defined(Symbols::TORUS_ACTION(), data.Q.path);
    call_and_discard("def " + Symbols::MOVING_CONE() + " = movingCone("
        + Symbols::TORUS_ACTION() + ");");
    createParent(file);
    write_ssi (Symbols::MOVING_CONE(), file.string());
  }

  void writeTrivialGroupActionToSSIFile
    ( int cardinality
    , const boost::filesystem::path& file
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    call_and_discard("def g = trivialGroupAction("
      + std::to_string(cardinality) + ");");
    createParent(file);
    write_ssi ("g", file.string());
    kill("g");
  }

  pnet_list findOrbitRepresentatives(const pnet_problem_data& data)
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    loadSymmetryGroup(data);
    call_and_discard("def orbits = findOrbitRepresentatives("
      + Symbols::SYMMETRY_GROUP() + "," + std::to_string(data.k) + ");");
    pnet_list orbits = toList(getList("orbits"));
    kill("orbits");
    return orbits;
  }

  pnet_list minimalOrbitRepresentativeOfGroupAction_TESTING_ONLY
    ( const pnet_list& representative
    , const boost::filesystem::path& groupActionSSIFile
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi("g", groupActionSSIFile.string());
    put("rep", createIntvec(representative));
    call_and_discard("def mRep = minimalRepresentative(rep,g);");
    const auto result = toList(singular::getIntvec("mRep"));
    kill("g", "rep", "mRep");
    return result;
  }

  pnet_list computeOrbitOfGroupAction_TESTING_ONLY
    ( const pnet_list& representative
    , const boost::filesystem::path& groupActionSSIFile
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi("g", groupActionSSIFile.string());
    put("rep", createIntvec(representative));
    call_and_discard("def orbit = computeOrbit(rep,g);");
    const auto result = toList(singular::getList("orbit"));
    kill("g", "rep", "orbit");
    return result;
  }

  ::gitfan::pnet_list computeOrbitOfSymmetryGroup
    ( const pnet_problem_data& data
    , const pnet_list& representative
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    loadSymmetryGroup(data);
    put("rep", createIntvec(representative));
    call_and_discard("def orbit = computeOrbit(rep," +
      Symbols::SYMMETRY_GROUP() + ");");
    const auto result = toList(singular::getList("orbit"));
    kill("rep", "orbit");
    return result;
  }

  std::vector<fs::path> computeOrbitConeOrbitWithDuplicatesAndWriteSSIFiles
    ( const pnet_problem_data& data
    , const pnet_file& representative
    , const fs::path& filePrefix
    )
  {
    createParent(filePrefix);
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    loadSymmetryGroupOnCones(data);
    load_ssi("rep", representative.path);
    call_and_discard("def orbit = computeOrbitConeOrbitWithDuplicates(rep,"
      + Symbols::SYMMETRY_GROUP_ON_CONES() + ");");
    int size = std::stoi(get_result("size(orbit);"));
    std::vector<fs::path> newFiles;
    for (int i = 1; i <= size; i++)
    {
      const std::string filePath =
        filePrefix.string() + "_" + std::to_string(i);
      write_ssi("orbit[" + std::to_string(i) + "]", filePath);
      newFiles.emplace_back(filePath);
    }
    kill("rep", "orbit");
    return newFiles;
  }

  // Note: ownership of faces is taken by this method.
  pnet_list computeFullDimAFaces(const pnet_problem_data& data, lists& faces)
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi_if_not_defined (Symbols::IDEAL(), data.ideal.path);
    load_ssi_if_not_defined (Symbols::TORUS_ACTION(), data.Q.path);
    put("L", faces);

    call_and_discard("def AF = computeFullDimAFaces(" + Symbols::IDEAL()
      + "," + Symbols::TORUS_ACTION() + ",L);");

    pnet_list result = toList(getList("AF"));
    kill("AF", "L");
    return result;
  }

  void computeOrbitConeAndWriteSSIFile
    ( const pnet_problem_data& data
    , const pnet_list& aface
    , const boost::filesystem::path& file
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi_if_not_defined(Symbols::TORUS_ACTION(), data.Q.path);
    put("aface", createIntvec(aface));
    call_and_discard("def c = computeOrbitConeForAFace("
      + Symbols::TORUS_ACTION() + ",aface);");
    createParent(file);
    write_ssi ("c", file.string());
    kill("aface", "c");
  }

  pnet_list intersectWithMovingConeAndOverwriteSSIFiles
    ( const pnet_problem_data& data
    , const pnet_list& orbitCones
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    loadMovingCone(data);
    load_ssi("ocs", orbitCones);
    call_and_discard("list result; list kept;"
      "result,kept = intersectOrbitConesWithMovingCone(ocs, "
      + Symbols::MOVING_CONE() + ");");

    pnet_list keptOrbitCones;
    // Note: Indices in kept are 1-based (as singular has 1-indexed lists).
    auto indexedOrbitCones = boost::range_detail::indexed_iterator
      <pnet_list::const_iterator>(orbitCones.cbegin(), 1);
    const pnet_list kept = toList(getList("kept"));
    for (const auto& keptIndex : kept
        | boost::adaptors::transformed(&as<int>)
        | boost::adaptors::indexed(1)
        )
    {
      std::advance(indexedOrbitCones,
                   keptIndex.value() - (*indexedOrbitCones).index());
      const pnet_value& keptOrbitCone = (*indexedOrbitCones).value();
      keptOrbitCones.push_back(keptOrbitCone);
      write_ssi("result[" + std::to_string(keptIndex.index()) + "]",
                pnetc::type::file::from_value(keptOrbitCone).path);
    }
    singular::kill("ocs", "result", "kept");
    return keptOrbitCones;
  }

  pnet_list computeInitialCone
    ( const pnet_problem_data& data
    , const pnet_list& orbitCones
    , const boost::filesystem::path& symmetryGroupSSIFile
    )
  {
    init();
    load_library(config::gitfanInterpreterLibrary().string());
    load_ssi_if_not_defined (Symbols::TORUS_ACTION(), data.Q.path);
    load_ssi_if_not_defined(Symbols::ORBIT_CONES(), orbitCones);
    load_ssi_if_not_defined( Symbols::SYMMETRY_GROUP_ON_HASHES()
                           , symmetryGroupSSIFile.string()
                           );
    loadMovingCone(data);
    call_and_discard("intvec hash = computeInitialConeHash("
      + Symbols::TORUS_ACTION() + ","
      + Symbols::MOVING_CONE() + ","
      + Symbols::ORBIT_CONES() + ");");
    call_and_discard("hash = minimalRepresentative(hash,"
      + Symbols::SYMMETRY_GROUP_ON_HASHES() + ");");
    pnet_list initialCone = conversion::toList(getIntvec("hash"));
    kill("hash");
    return initialCone;
  }

  pnet_list_of_sets computeNeighbourHashes
    ( const pnet_problem_data& data
    , const pnet_list& orbitCones
    , const fs::path& symmetryGroupSSIFile
    , const pnet_list& conesToExpand
    )
  {
    if (conesToExpand.empty())
    {
      return pnet_list_of_sets();
    }

    init();
    load_library(config::gitfanInterpreterLibrary().string());
    loadMovingCone(data);
    load_ssi_if_not_defined( Symbols::SYMMETRY_GROUP_ON_HASHES()
                           , symmetryGroupSSIFile.string()
                           );
    load_ssi_if_not_defined(Symbols::ORBIT_CONES(), orbitCones);

    pnet_list_of_sets result;
    for (const pnet_list& cone : conesToExpand
        | boost::adaptors::transformed (&as<pnet_list>)
        )
    {
      put("hash", createIntvec(cone));
      call_and_discard("list neighbours = computeNeighbourHashes(hash,"
          + Symbols::ORBIT_CONES() + "," + Symbols::MOVING_CONE() + ");");
      call_and_discard("neighbours = toMinimalRepresentatives(neighbours,"
        + Symbols::SYMMETRY_GROUP_ON_HASHES() + ");"
        );
      pnet_set neighbour_set;
      insert( getList("neighbours")
            , std::inserter(neighbour_set, neighbour_set.begin())
            );
      result.push_back(neighbour_set);
      kill("hash", "neighbours");
    }
    return result;
  }
}
}

namespace std
{
  singular::gitfan::Permutation operator*
    ( const singular::gitfan::Permutation& l
    , const singular::gitfan::Permutation& r
    )
  {
    singular::gitfan::Permutation result;
    result.reserve(l.size());
    for (unsigned int i = 0; i < l.size(); i++)
    {
      result.push_back(l[r[i]]);
    }
    return result;
  }
}
