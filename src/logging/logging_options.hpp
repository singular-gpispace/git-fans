#pragma once

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace gitfan {
namespace logging
{
  class LoggingOptions
  {
    public:
      LoggingOptions();
      LoggingOptions(const std::vector<std::string>& options);
      LoggingOptions(const std::string& serialized);

      std::string serialize() const;

      ::boost::optional<::boost::filesystem::path> logsDirectory;
  };
}
}
