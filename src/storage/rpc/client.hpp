#pragma once

#include <storage/options.hpp>
#include <storage/conestorage.hpp>
#include <storage/rpc/protocol.hpp>

#include <rpc/locked_with_info_file.hpp>
#include <rpc/remote_function.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

namespace gitfan
{
  namespace storage
  {
    namespace rpc
    {
      struct Client : public fhg::util::
                        scoped_boost_asio_io_service_with_threads
                    , public fhg::rpc::locked_with_info_file::client
                    , public IConeStorage
      {
        inline Client (const Options& options)
          : fhg::util::scoped_boost_asio_io_service_with_threads (1)
          , fhg::rpc::locked_with_info_file::client
            (*this, options.workingDirectory)
        {}

#define FUNCTION(_name)                                                 \
        fhg::rpc::sync_remote_function<protocol::_name> _name = {*this}

        FUNCTION (put);
        FUNCTION (finish);
        FUNCTION (get);
        FUNCTION (save);

#undef FUNCTION

        virtual inline bool addMarkedCone (const Cone& cone) override
        {
          return put (cone);
        }
        virtual inline void endOfTransaction (const Cone& cone) override
        {
          finish (cone);
        }
        virtual inline std::vector<Cone> unmarkCones (std::size_t n) override
        {
          return get (n);
        }
        virtual inline bool saveConesPersistently
         (const boost::filesystem::path& file) override
        {
          return save (file);
        }
      };
    }
  }
}
