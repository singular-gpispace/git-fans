#pragma once

#include <util-gitfan/type_aliases.hpp>
#include <util-gitfan/singular_library_wrapper.hpp>

#include <boost/filesystem.hpp>

#include <vector>
#include <functional>

namespace gitfan {
namespace testing {
namespace util
{
  typedef singular::gitfan::Permutation permutation;

  bool ssiFilesExistWithIdenticalContent(const boost::filesystem::path& p1,
                                      const boost::filesystem::path& p2);

  bool fileIsEmpty(const boost::filesystem::path& pFile);

  template<typename T>
  void read_workflow_result(
    const std::multimap<std::string, pnet_value>& results,
    const std::string& port,
    T& result);

  template<typename T>
  void expect_workflow_result(
    const std::multimap<std::string, pnet_value>& results,
    const std::string& port,
    const T& expectedResult);

  template< typename FromContainer
          , typename ToContainer
          , typename BinaryPredicate
          >
  permutation computePermutation
    ( const FromContainer& from
    , const ToContainer& to
    , const BinaryPredicate& equal
    );

  template< typename FromContainer
          , typename ToContainer
          , typename BinaryPredicate
          >
  bool isPermutation
    ( const FromContainer& from
    , const ToContainer& to
    , const BinaryPredicate& equal
    );

  void applyPermutationToHash(const permutation& sigma, pnet_list& hash);

  void applyPermutationToGroupAction(
    const permutation& sigma,
    singular::gitfan::GroupAction& action);

  pnet_list toPnetList(const std::vector<boost::filesystem::path>& paths);

  std::vector<boost::filesystem::path> toFilePaths(const pnet_list& paths);

  std::vector<boost::filesystem::path> loadOrbitConeListFile
    (const boost::filesystem::path& path);

  void copyDirectory(
    boost::filesystem::path source,
    boost::filesystem::path dest);

  template <typename T>
  const std::set<T> asSet(const std::list<T>& list);

  template <typename T>
  const std::set<T> asSet(const std::vector<T>& list);

}}}

#include "util.hxx"
