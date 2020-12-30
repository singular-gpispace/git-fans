#pragma once

#include <vector>
#include <boost/filesystem.hpp>

namespace gitfan {
namespace storage
{
  class Options
  {
    public:
      Options();
      Options(const std::vector<std::string>& options,
              const boost::filesystem::path& workingDirectory);
      Options(const std::string& serialized);

      std::string serialize() const;

      std::string implementationTag;
      unsigned long subdirectoryNameLength;
      boost::filesystem::path workingDirectory;
  };
}
}
