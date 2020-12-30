#include <storage/filesystem/string_container.hpp>

#include <boost/system/error_code.hpp>

#include <stdexcept>

namespace fs = boost::filesystem;

namespace gitfan {
namespace storage {
namespace filesystem
{
  StringContainer::iterator::iterator
    (const StringContainer* container)
    : _internalIterator(container->_base)
    , _container(container)
  {
    moveToValidPosition();
  }

  bool StringContainer::iterator::operator!=(const iterator& other) const
  {
    return _internalIterator != other._internalIterator;
  }
  bool StringContainer::iterator::operator==(const iterator& other) const
  {
    return _internalIterator == other._internalIterator;
  }
  StringContainer::iterator& StringContainer::iterator::operator++()
  {
    ++_internalIterator;
    moveToValidPosition();
    return *this;
  }
  std::string StringContainer::iterator::operator*() const
  {
    return _container->_asString(_internalIterator->path());
  }

  void StringContainer::iterator::moveToValidPosition()
  {
    while(!isAtValidPosition())
    {
      ++_internalIterator;
    }
  }

  bool StringContainer::iterator::isAtValidPosition()
  {
    if (_internalIterator == fs::recursive_directory_iterator())
    {
      return true;
    }
    const std::string pathString = _internalIterator->path().string();
    return !pathString.empty() && pathString.back() == _container->_suffix;
  }

  StringContainer::StringContainer
    ( fs::path base
    , size_t subdirectoryNameLength
    )
    : _suffix('$')
    , _base(std::move(base))
    , _subdirectoryNameLength(std::move(subdirectoryNameLength))
  {
    fs::create_directories(_base);
  }

  void createDirectoryRecursive(const fs::path& base, const fs::path& path)
  {
    if (path.empty())
    {
      return;
    }
    bool failed = true;
    while (failed)
    {
      failed = false;
      createDirectoryRecursive(base, path.parent_path());
      try
      {
        fs::create_directory(base / path);
      }
      catch(fs::filesystem_error& e)
      {
        if (e.code() != boost::system::errc::no_such_file_or_directory)
        {
          throw;
        }
        failed = true;
      }
    }
  }

  bool StringContainer:: add(const std::string& str)
  {
    fs::path path = _asPath(str);
    bool directoryCreated = false;
    bool failed = true;
    while (failed)
    {
      failed = false;
      createDirectoryRecursive(_base, path.parent_path());
      try
      {
        directoryCreated = fs::create_directory(_base / path);
      }
      catch(fs::filesystem_error& e)
      {
        if (e.code() != boost::system::errc::no_such_file_or_directory)
        {
          throw;
        }
        failed = true;
      }
    }
    return directoryCreated;
  }

  void removeIfEmpty(const fs::path& base, const fs::path& path)
  {
    if(path.empty())
    {
      return;
    }
    try
    {
      fs::remove(base / path);
    }
    catch(fs::filesystem_error& e)
    {
      if (e.code() != boost::system::errc::directory_not_empty)
      {
        throw;
      }
    }
    removeIfEmpty(base, path.parent_path());
  }

  bool StringContainer:: remove(const std::string& str)
  {
    fs::path path = _asPath(str);
    bool success = fs::remove(_base / path);
    removeIfEmpty(_base, path.parent_path());
    return success;
  }

  StringContainer::iterator StringContainer::begin() const
  {
    return StringContainer::iterator(this);
  }

  StringContainer::iterator StringContainer::end() const
  {
    return StringContainer::iterator();
  }

  std::string StringContainer::_asString(const fs::path& path) const
  {
    std::string pathString(path.string());
    pathString.erase(0, _base.string().size());
    pathString.erase(pathString.size() - 1, 1);
    for (size_t pos = pathString.find('/');
         pos != std::string::npos;
         pos = pathString.find('/'))
    {
      pathString.erase(pos, 1);
    }
    return pathString;
  }

  fs::path StringContainer::_asPath(const std::string& str) const
  {
    for (const char& c : str)
    {
      if (c == '/' || c == '\\' || c == _suffix)
      {
        throw std::runtime_error
          ("StringContainer does not accept strings containing "
           "'/', '\\' or '" + std::to_string(_suffix)
           + "'. (string: " + str + ")");
      }
    }
    fs::path path(str.substr(0, _subdirectoryNameLength));
    for ( size_t pos = _subdirectoryNameLength
        ; pos < str.size()
        ; pos += _subdirectoryNameLength
        )
    {
      path /= str.substr(pos, _subdirectoryNameLength);
    }
    path += _suffix;
    return path;
  }
}
}
}
