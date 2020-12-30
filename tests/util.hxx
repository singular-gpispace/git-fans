#include <gtest/gtest.h>

#include <util-gitfan/type_aliases.hpp>

#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <sstream>

namespace gitfan {
namespace testing {
namespace util
{
  template<typename T>
  void read_workflow_result(
    const std::multimap<std::string, pnet_value>& results,
    const std::string& port,
    T& result)
  {
    const auto& elem = results.find(port);
    ASSERT_NE(results.end(), elem)
      << "No result returned on port \"" << port << "\".";
    result = as<T>(elem->second);
  }

  template<typename T>
  void expect_workflow_result(
    const std::multimap<std::string, pnet_value>& results,
    const std::string& port,
    const T& expectedResult)
  {
    T actualResult;
    read_workflow_result(results, port, actualResult);
    EXPECT_EQ(expectedResult, actualResult)
      << "Bad result returned on port \"" << port << "\".";
  }

  template< typename FromContainer
          , typename ToContainer
          , typename BinaryPredicate
          >
  permutation computePermutation
    ( const FromContainer& from
    , const ToContainer& to
    , const BinaryPredicate& equal
    )
  {
    if (from.size() != to.size())
    {
      std::stringstream s;
      s << "\"to\" and \"from\" do not have the same cardinality. ("
        << from.size() << "!=" << to.size() << ")";
      throw std::runtime_error(s.str());
    }
    permutation permutation(from.size());

    // schwartz transformation, after Randal L. Schwartz
    std::unordered_map<int, typename ToContainer::const_iterator> idx;
    {
      auto t(to.begin());
      for (unsigned int i = 0; i < from.size(); ++i, ++t)
      {
        idx.emplace (i, t);
      }
    }
    {
      auto x(from.begin());
      for (unsigned int i = 0; i < from.size(); ++i, ++x)
      {
        bool found = false;
        for (auto t : idx)
        {
          if (equal(*x, *t.second))
          {
            permutation[i] = t.first;
            idx.erase(t.first);
            found = true;
            break;
          }
        }
        if (!found)
        {
          std::stringstream s;
          s << "Missing image for from[" << i << "]=" << *x << ".";
          throw std::runtime_error(s.str());
        }
      }
    }
    return permutation;
  }

  template< typename FromContainer
          , typename ToContainer
          , typename BinaryPredicate
          >
  bool isPermutation
    ( const FromContainer& from
    , const ToContainer& to
    , const BinaryPredicate& equal
    )
  {
    try
    {
      computePermutation(from, to, equal);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  template <typename T>
  const std::set<T> asSet(const std::list<T>& list)
  {
    return std::set<T>(list.begin(), list.end());
  }

  template <typename T>
  const std::set<T> asSet(const std::vector<T>& list)
  {
    return std::set<T>(list.begin(), list.end());
  }

}}}
