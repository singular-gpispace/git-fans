#include <storage/options.hpp>
#include <storage/conestorage.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/program_options.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/join.hpp>

#include <sstream>

namespace gitfan {
namespace storage
{
  Options::Options() :
    Options ( std::vector<std::string>()
            , boost::filesystem::current_path() / "storage"
            )
  {}

  Options::Options
    (const std::vector<std::string>& options,
     const boost::filesystem::path& pWorkingDirectory)
  {
    namespace po = fhg::util::boost::program_options;

    const auto implementationTags = IConeStorage::availableImplementations();
    const po::option<po::nonempty_string>
      implementationOption
        { "implementation"
        , "Specifies the cone storage implementation used by the algorithm "
          "(one of " + fhg::util::join(implementationTags, ", ").string() + ")"
        , IConeStorage::defaultImplementation()
        };

    const po::option<po::positive_integral<unsigned long>>
      subdirectoryNameLengthOption
        { "subdirectory-name-length"
        , "maximal length of a folder name created by FileConeStorage"
        , 3
        };

    boost::program_options::variables_map const vm
      (fhg::util::boost::program_options::options ("ConeStorage")
      .add (implementationOption)
      .add (subdirectoryNameLengthOption)
      .store_and_notify (options)
      );

    implementationTag = implementationOption.get_from(vm);
    subdirectoryNameLength = subdirectoryNameLengthOption.get_from(vm);
    workingDirectory = pWorkingDirectory;

    if (std::find( implementationTags.begin()
                 , implementationTags.end()
                 , implementationTag
                 ) == implementationTags.end())
    {
      throw boost::program_options::invalid_option_value
                ("Unknown implementation " + implementationTag);
    }
  }

  Options::Options(const std::string& serialized)
  {
    std::istringstream iss (serialized);
    boost::archive::text_iarchive ia (iss);
    ia & implementationTag;
    ia & subdirectoryNameLength;
    ia & workingDirectory;
  }

  std::string Options::serialize() const
  {
    std::ostringstream oss;
    boost::archive::text_oarchive oa (oss);
    oa & implementationTag;
    oa & subdirectoryNameLength;
    oa & workingDirectory;
    return oss.str();
  }
}
}
