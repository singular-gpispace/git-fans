#include <logging/command_line_args_printer.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

#include <sstream>


namespace fs = boost::filesystem;

namespace gitfan {
namespace logging
{
  CommandLineArgsPrinter::CommandLineArgsPrinter(int argc, char** argv)
  {
    std::stringstream sstream;
    for (int i = 0; i < argc; i++)
    {
      sstream << "argv[" << i << "]: " << argv[i] << std::endl;
    }
    std::string str = sstream.str();
    _args = str.substr(0, str.size() - 1);
  }

  void CommandLineArgsPrinter::printToFile(const fs::path& file)
  {
    if (file.has_parent_path())
    {
      fs::create_directories(file.parent_path());
    }
    fs::ofstream o(file, std::ios_base::out | std::ios_base::app);
    o << _args;
  }
}
}
