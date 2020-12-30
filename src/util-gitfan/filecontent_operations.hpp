#pragma once

#include <util-gitfan/lru_cache.hpp>

#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

#include <functional>
#include <memory>

namespace std
{
  template<>
  struct hash<boost::filesystem::path>
  {
    size_t operator()(const boost::filesystem::path& p) const
    {
      return boost::filesystem::hash_value(p);
    }
  };
}

namespace gitfan {
namespace filecontent
{
  typedef LRUCache<boost::filesystem::path, std::string> Cache;
  struct Hash
  {
    inline Hash() {}
    inline Hash(const std::shared_ptr<Cache>& cache) : _cache(cache) {}

    std::size_t operator()(const boost::filesystem::path& file) const;
  private:
    std::hash<std::string> _hash;
    std::shared_ptr<Cache> _cache;
  };

  struct EqualTo
  {
    inline EqualTo() {}
    inline EqualTo(const std::shared_ptr<Cache>& cache) : _cache(cache) {}

    bool operator()( const boost::filesystem::path& lhs
                   , const boost::filesystem::path& rhs
                   ) const;
  private:
    std::shared_ptr<Cache> _cache;
  };

  struct Compare
  {
    inline Compare() {}
    inline Compare(const std::shared_ptr<Cache>& cache) : _cache(cache) {}

    bool operator()( const boost::filesystem::path& lhs
                   , const boost::filesystem::path& rhs
                   ) const;
  private:
    std::shared_ptr<Cache> _cache;
  };
}
}
