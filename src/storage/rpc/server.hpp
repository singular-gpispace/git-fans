#pragma once

#include <storage/service_provider.hpp>
#include <storage/rpc/protocol.hpp>

#include <rpc/locked_with_info_file.hpp>
#include <rpc/service_handler.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <boost/filesystem/fstream.hpp>

#include <exception>
#include <unordered_set>
#include <chrono>

namespace gitfan
{
  namespace storage
  {
    namespace rpc
    {
      namespace fs = ::boost::filesystem;

      //! \note NOT thread safe
      class Server : public ServiceProvider::Implementation
      {
      public:
        Server (const Options& options);
        ~Server() noexcept;

      private:
        std::unordered_set<Cone> _marked;
        std::unordered_set<Cone> _unmarked;
        fhg::util::filesystem_lock_directory _lock;
        fhg::rpc::service_dispatcher _service_dispatcher;
        fhg::rpc::service_handler<protocol::put> _put;
        fhg::rpc::service_handler<protocol::finish> _finish;
        fhg::rpc::service_handler<protocol::get> _get;
        fhg::rpc::service_handler<protocol::save> _save;
        fhg::util::scoped_boost_asio_io_service_with_threads _io_service;
        fhg::rpc::locked_with_info_file::server const _rpc_server;

        std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
        _fan_evolution_stream;
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
        _marked_out_stream;
        std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
        _unmarked_out_stream;
        std::vector<std::size_t> _pending_evolution_values;
        std::vector<Cone> _pending_marked_values;
        std::vector<Cone> _pending_unmarked_values;
        std::chrono::steady_clock::time_point _last_flush_time;

        void flushPending();
      };
    }
  }
}
