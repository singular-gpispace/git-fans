#include <storage/service_provider.hpp>

#include <storage/rpc/server.hpp>


#include <memory>

namespace gitfan
{
  namespace storage
  {
    namespace
    {
      typedef std::function<std::unique_ptr<ServiceProvider::Implementation>
        (const Options&)> ImplFactoryFunction;

      const std::map<std::string, ImplFactoryFunction>& factory()
      {
        static const std::map<std::string, ImplFactoryFunction> _
          { { "rpc"
            , [](const Options& o)
                -> std::unique_ptr<ServiceProvider::Implementation>
              {
                return std::make_unique<rpc::Server> (o);
              }
            }
          };
        return _;
      }
    }

    ServiceProvider::ServiceProvider(const Options& options)
      : _options(options)
      , _impl(factory().find(options.implementationTag) == factory().end()
             ? std::make_unique<ServiceProvider::Implementation>()
             : factory().at(options.implementationTag)(options)
             )
    {}
  }
}
