#include <gtest/gtest.h>

#include <util-gitfan/lru_cache.hpp>

namespace gitfan {
namespace testing
{
  typedef LRUCache<std::string, std::string> LRUCache;

  std::string addUnderscore(const std::string& key)
  {
    return key + "_";
  }

  std::string returnDash(const std::string& key)
  {
    return "-";
  }

  TEST(LRUCacheTest, ZeroSizeCacheDoesNotCache)
  {
    LRUCache cache(0);

    EXPECT_EQ("test_", cache.get("test", &addUnderscore));
    EXPECT_EQ("-", cache.get("test", &returnDash));
  }

  TEST(LRUCacheTest, cachesOneElement)
  {
    LRUCache cache(5);
    EXPECT_EQ("test_", cache.get("test", &addUnderscore));
    EXPECT_EQ("test_", cache.get("test", &returnDash));
  }

  TEST(LRUCacheTest, ThreeSizedCacheCachesExactlyThreeElements)
  {
    LRUCache cache(3);
    EXPECT_EQ("test1_", cache.get("test1", &addUnderscore));
    EXPECT_EQ("test2_", cache.get("test2", &addUnderscore));
    EXPECT_EQ("test3_", cache.get("test3", &addUnderscore));
    EXPECT_EQ("test4_", cache.get("test4", &addUnderscore));

    EXPECT_EQ("test2_", cache.get("test2", &returnDash));
    EXPECT_EQ("test3_", cache.get("test3", &returnDash));
    EXPECT_EQ("test4_", cache.get("test4", &returnDash));
    EXPECT_EQ("-", cache.get("test1", &returnDash));
  }

  TEST(LRUCacheTest, removesLeastRecentlyUsedEntry)
  {
    LRUCache cache(2);
    EXPECT_EQ("test1_", cache.get("test1", &addUnderscore));
    EXPECT_EQ("test2_", cache.get("test2", &addUnderscore));
    EXPECT_EQ("test1_", cache.get("test1", &returnDash));
    EXPECT_EQ("test3_", cache.get("test3", &addUnderscore));
    EXPECT_EQ("-", cache.get("test2", &returnDash));
  }
}
}
