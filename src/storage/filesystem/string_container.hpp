#pragma once

#include <boost/filesystem.hpp>

namespace gitfan {
namespace storage {
namespace filesystem
{
  class StringContainer
  {

  public:

    class iterator
    {
    public:
      iterator() = default;
      iterator(const StringContainer* container);

      bool operator!=(const iterator& other) const;
      bool operator==(const iterator& other) const;
      iterator& operator++();
      std::string operator*() const;
    private:
      void moveToValidPosition();
      bool isAtValidPosition();

      boost::filesystem::recursive_directory_iterator _internalIterator;
      const StringContainer* _container;
    };

    friend class iterator;

    StringContainer
      ( boost::filesystem::path base
      , size_t subdirectoryNameLength
      );

    bool add(const std::string& str);

    // Invalidates iterators
    bool remove(const std::string& str);

    iterator begin() const;
    iterator end() const;

  private:
    const char _suffix;
    const boost::filesystem::path _base;
    const size_t _subdirectoryNameLength;

    std::string _asString(const boost::filesystem::path& path) const;
    boost::filesystem::path _asPath(const std::string& str) const;
  };
}
}
}
