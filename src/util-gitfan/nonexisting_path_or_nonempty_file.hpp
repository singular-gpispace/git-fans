#pragma once

#include <util-generic/boost/program_options/validators.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace gitfan {
namespace boost {
namespace program_options
{
  struct nonexisting_path_or_nonempty_file
    : public ::boost::filesystem::path
  {
    nonexisting_path_or_nonempty_file (std::string const& option)
      : ::boost::filesystem::path(::boost::filesystem::path(option).lexically_normal())
    {
      if (::boost::filesystem::exists (*this))
      {
        if (!::boost::filesystem::is_regular_file(*this))
        {
          throw ::boost::program_options::invalid_option_value
            ((::boost::format ("The existing path %1% is no regular file.")
               % *this).str());
        }
        if (!(::boost::filesystem::file_size(*this) > 0))
        {
          throw ::boost::program_options::invalid_option_value
             ((::boost::format ("%1% has size zero.") % *this).str());
        }
      }
    }
  };

  inline void validate ( ::boost::any& v
                         , const std::vector<std::string>& values
                         , nonexisting_path_or_nonempty_file*
                         , int
                         )
  {
    fhg::util::boost::program_options::validate
      <nonexisting_path_or_nonempty_file> (v, values);
  }

}}}
