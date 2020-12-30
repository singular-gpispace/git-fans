#pragma once

#include <storage/options.hpp>

#include <boost/filesystem/path.hpp>

#include <memory>

namespace gitfan
{
  namespace storage
  {
    class ServiceProvider
    {
    public:
      class Implementation
      {
      public:
        virtual ~Implementation() = default;
      };

      ServiceProvider (const Options& options);

      inline Options const& options() const
      {
        return _options;
      }

    private:
      const Options _options;
      std::unique_ptr<Implementation> _impl;
    };
  }
}
