#include "logging_options.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/optional.hpp>

#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <sstream>

namespace gitfan {
namespace logging
{
  LoggingOptions::LoggingOptions() : LoggingOptions(std::vector<std::string>())
  {}

  LoggingOptions::LoggingOptions(const std::vector<std::string>& options)
  {
    namespace po = fhg::util::boost::program_options;

    const po::option<po::nonexisting_path> logsOption
      { "logs"
      , "A nonexisting directory in which log files are placed. "
        "If this option is missing, the log output is redirected to standard "
        "output."
      };

    boost::program_options::variables_map const vm
      (fhg::util::boost::program_options::options ("Logging")
      .add (logsOption)
      .store_and_notify (options)
      );

    logsDirectory = logsOption.get<::boost::filesystem::path>(vm);
  }

  LoggingOptions::LoggingOptions(const std::string& serialized)
  {
    std::istringstream iss (serialized);
    boost::archive::text_iarchive ia (iss);
    ia & logsDirectory;
  }

  std::string LoggingOptions::serialize() const
  {
    std::ostringstream oss;
    boost::archive::text_oarchive oa (oss);
    oa & logsDirectory;
    return oss.str();
  }
}
}
