#ifndef GITFAN_INTERFACE_H
#define GITFAN_INTERFACE_H

#define NO_NAME_MANGLING extern "C"

#include "type_aliases.hpp"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <util-generic/dynamic_linking.hpp>

#include <storage/options.hpp>

#include <config.hpp>

#include <functional>

#define RESOLVE_INTERFACE_FUNCTION(function) \
  ::gitfan::interface::resolve<decltype(::gitfan::interface::function)> \
    (BOOST_PP_STRINGIZE(function))

namespace gitfan {
namespace interface
{
  template <typename T>
  std::function<T> resolve(const std::string& symbol)
  {
    return fhg::util::scoped_dlhandle
      (config::gitfanInterfaceLibrary(), RTLD_GLOBAL | RTLD_NOW | RTLD_DEEPBIND)
      .sym<T>(symbol);
  }

  NO_NAME_MANGLING
  storage::Options initStorageOptions
    (const std::vector<std::string>& options,
     const boost::filesystem::path& workingDirectory);

  NO_NAME_MANGLING
  void processPrecomputedData
    (const pnet_problem_data& data,
     const pnet_directory& workingDirectory,
     pnet_list& orbitCones,
     pnet_file& symmetryGroupOnHashesFile);

  NO_NAME_MANGLING
  pnet_file removeOrbitStructureAndReturnSymmetryGroupOnHashesFile
    (const pnet_problem_data& data,
     const pnet_directory& workingDirectory,
     pnet_list& orbitInfos);

  NO_NAME_MANGLING
  void preprocessProblemData
    (pnet_problem_data& data,
     const pnet_directory& workingDirectory);

  NO_NAME_MANGLING
  pnet_list_range createSimplexFaces
    (int r,
     int k,
     pnet_list_range& last,
     unsigned long range,
     bool& finished);

  NO_NAME_MANGLING
  pnet_list_range createSimplexFacesFromFile
    (const pnet_problem_data& data,
     pnet_list_range& last,
     unsigned long range,
     bool& finished);

  NO_NAME_MANGLING
  pnet_list computeFullDimAFacesFromRange
    (const pnet_problem_data& data,
     const pnet_list_range& simplexFaces);

  NO_NAME_MANGLING
  pnet_list computeFullDimAFacesFromFile
    (const pnet_problem_data& data,
     const pnet_list_range& simplexFaces);

  NO_NAME_MANGLING
  pnet_file computeOrbitCone
    (const pnet_list& aface,
     const pnet_problem_data& data,
     const pnet_directory& workingDirectory);

  NO_NAME_MANGLING
  pnet_list intersectWithMovingCone
    (const pnet_problem_data& data,
     const pnet_list& orbitCones);

  NO_NAME_MANGLING
  bool isLastOfAllEqualOrbitConeOrbits
    (const pnet_problem_data& data,
     const pnet_list& orbitInfos,
     pnet_list::const_iterator orbitInfo);

  NO_NAME_MANGLING
  pnet_map expandOrbitConeOrbit
    (const pnet_problem_data& data,
     const pnet_directory& workingDirectory,
     const pnet_file& representative,
     unsigned long orbitNumber);

  NO_NAME_MANGLING
  void removeIntermediateOrbitInfo(const pnet_directory& workingDirectory);

  NO_NAME_MANGLING
  boost::optional<pnet_list> computeInitialCone
    (const pnet_problem_data& data,
     const pnet_list& orbitCones,
     const pnet_file& symmetryGroupSSIFile);

  NO_NAME_MANGLING
  pnet_list getExpandableCones
    (const std::string& storageOptions,
     const unsigned long& jobSize);

  NO_NAME_MANGLING
  pnet_list_of_sets expandCones
    (const pnet_problem_data& data,
     const pnet_list& orbitCones,
     const pnet_file& symmetryGroupSSIFile,
     const pnet_list& conesToExpand);

  NO_NAME_MANGLING
  unsigned long insertNeighbours
    (const std::string& storageOptions,
     const unsigned long& numberOfOrbitCones,
     const pnet_list_of_sets& neighbours,
     const pnet_list& expandedCones);

  NO_NAME_MANGLING
  void outputOrbitConesOrder
    (const pnet_directory& workingDirectory,
     const pnet_list& orbitCones);

  NO_NAME_MANGLING
  void outputGitCones
    (const pnet_directory& workingDirectory,
     const std::string& storageOptions);

  NO_NAME_MANGLING
  void outputLoadGitConesSingularFile
    (const pnet_directory& workingDirectory,
     const pnet_list& orbitCones);
}}


#endif
