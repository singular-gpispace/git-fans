#pragma once

#include <drts/drts.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

namespace gitfan
{
  //! \note collects information relative to the path of the installation
  class installation
  {
  public:
    installation (boost::program_options::variables_map const&,
                  const boost::filesystem::path& installPath);

    gspc::installation gspc_installation() const;
    boost::filesystem::path workflow() const;

  private:
    boost::filesystem::path const _path;
  };
}
