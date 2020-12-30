#pragma once

#include <boost/filesystem.hpp>
#include <main.hpp>
#include <iostream>

namespace gitfan {
namespace testing {
namespace util
{
  class TemporaryUniquePath
  {
    public:

      TemporaryUniquePath(const std::string& prefix)
        : _path(boost::filesystem::unique_path(prefix + "_%%%%-%%%%-%%%%-%%%%"))
      {
      }

      ~TemporaryUniquePath()
      {
        if (!containedInArgs("--keep-temporary-files"))
        {
          boost::filesystem::remove_all (_path);
        }
      }

      inline std::string string() const
      {
        return _path.string();
      }

      inline const char* c_str() const
      {
        return _path.c_str();
      }

      operator boost::filesystem::path() const
      {
        return _path;
      }

      std::ostream& operator<<(std::ostream& o) const
      {
        o << _path;
        return o;
      }

    private:
      boost::filesystem::path _path;
  };

}}}

