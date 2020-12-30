#pragma once

#include <boost/filesystem.hpp>

#include <string>

namespace gitfan {
namespace logging
{
  class CommandLineArgsPrinter
  {
  public:
    CommandLineArgsPrinter(int argc, char** argv);

    void printToFile(const ::boost::filesystem::path& file);
  private:
    std::string _args;
  };
}
}
