#include <storage/conestorage.hpp>
#include <storage/filesystem/linear_conestorage.hpp>
#include <storage/filesystem/constant_conestorage.hpp>
#include <storage/rpc/client.hpp>



#include <map>
#include <memory>
#include <vector>

using namespace std;
namespace fs = boost::filesystem;

namespace gitfan {
namespace storage
{
  typedef std::function<std::unique_ptr<IConeStorage>
    (const Options&)> ConeStorageFactoryFunction;

  const std::map<std::string, ConeStorageFactoryFunction>& factory()
  {
    static const std::map<std::string, ConeStorageFactoryFunction> _
      { { "linear"
        , [](const Options& o) -> std::unique_ptr<IConeStorage>
          {
            return std::make_unique
              <filesystem::LinearConeStorage>(o);
          }
        }
      , { "constant"
        , [](const Options& o) -> std::unique_ptr<IConeStorage>
          {
            return std::make_unique
              <filesystem::ConstantConeStorage>(o);
          }
        }
      , { "rpc"
        , [](const Options& o) -> std::unique_ptr<IConeStorage>
          {
            return std::make_unique
              <rpc::Client> (o);
          }
        }
      };
    return _;
  }

  std::string defaultTag()
  {
    return "rpc";
  }

  std::string IConeStorage::defaultImplementation()
  {
    return defaultTag();
  }

  std::vector<std::string> IConeStorage::availableImplementations()
  {
    std::vector<std::string> tags;
    for (const auto& entry : factory())
    {
      tags.emplace_back(entry.first);
    }
    return tags;
  }

  std::unique_ptr<IConeStorage> IConeStorage::build(const std::string& options)
  {
    Options parsedOptions(options);
    return factory().at(parsedOptions.implementationTag)(parsedOptions);
  }
}
}
