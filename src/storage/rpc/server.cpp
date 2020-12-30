#include <storage/rpc/server.hpp>

#include <boost/algorithm/string/join.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace gitfan {
namespace storage {
namespace rpc
{
  namespace fs = boost::filesystem;

  Server::Server(const Options& options)
    : _lock (options.workingDirectory)
    , _service_dispatcher
       (fhg::util::serialization::exception::serialization_functions())
    , _put ( _service_dispatcher
           , [this] (Cone const& cone) -> bool
             {
               if (_unmarked.count (cone) == 0)
               {
                 _pending_marked_values.emplace_back(cone);
                 return _marked.emplace (cone).second;
               }
               else return false;
             }
           )
    , _finish ( _service_dispatcher
           , [this] (Cone const& cone) -> void
             {
               _pending_unmarked_values.emplace_back(cone);
               _pending_evolution_values.push_back(_unmarked.size() + _marked.size());
               std::chrono::duration<double> span =
                 std::chrono::steady_clock::now() - _last_flush_time;
               if (span.count() > 10)
               {
                 flushPending();
                 _last_flush_time = std::chrono::steady_clock::now();
               }
             }
           )
    , _get ( _service_dispatcher
           , [this] (std::size_t n) -> std::vector<Cone>
             {
                std::vector<Cone> cones;

                while (!_marked.empty() && n --> 0)
                {
                  auto const pos(_unmarked.emplace (*_marked.begin()));

                  if (!pos.second)
                  {
                    throw std::logic_error("double move to unmark");
                  }

                  cones.emplace_back (*pos.first);
                  _marked.erase (_marked.begin());
                }

                return cones;
              }
            )
    , _save ( _service_dispatcher
            , [this] (const fs::path& file) -> bool
              {
                if (!file.parent_path().empty())
                {
                  fs::create_directories(file.parent_path());
                }
                fs::ofstream stream (file);

                for (auto const& cone : _marked)
                {
                  stream << cone << '\n';
                }
                for (auto const& cone : _unmarked)
                {
                  stream << cone << '\n';
                }

                return stream.good();
              }
            )
    , _io_service (1)
    , _rpc_server (_io_service, _service_dispatcher, _lock)
    , _last_flush_time(std::chrono::steady_clock::now())
  {
    auto evolution_path (options.workingDirectory);
    auto marked_path (options.workingDirectory);
    auto unmarked_path (options.workingDirectory);
    evolution_path += "-fan_evolution.bin";
    marked_path += "-marked_cones.txt";
    unmarked_path += "-unmarked_cones.txt";
    _fan_evolution_stream =
          std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
            ( new fs::ofstream(evolution_path
                               , std::ios_base::out
                               | std::ios_base::app
                               | std::ios_base::binary
                               )
            , [](std::ostream* p) { delete p; }
            );
    _marked_out_stream =
          std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
            ( new fs::ofstream(marked_path
                               , std::ios_base::out
                               | std::ios_base::app
                               | std::ios_base::binary
                               )
            , [](std::ostream* p) { delete p; }
            );
    _unmarked_out_stream =
          std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
            ( new fs::ofstream(unmarked_path
                               , std::ios_base::out
                               | std::ios_base::app
                               )
            , [](std::ostream* p) { delete p; }
            );
  }

  Server::~Server()
  {
    try { flushPending(); } catch (...) {}
  }

  void Server::flushPending()
  {
    if (!_pending_evolution_values.empty())
    {
      const std::size_t* data = _pending_evolution_values.data();
      _fan_evolution_stream->write( reinterpret_cast<char const*>(data)
                                  , _pending_evolution_values.size() * sizeof(std::size_t)
                                  );
      _fan_evolution_stream->flush();
      _pending_evolution_values.clear();
    }
    if (!_pending_marked_values.empty())
    {
      *_marked_out_stream << boost::algorithm::join(_pending_marked_values, "\n") << std::endl;
      _pending_marked_values.clear();
    }
    if (!_pending_unmarked_values.empty())
    {
      *_unmarked_out_stream << boost::algorithm::join(_pending_unmarked_values, "\n") << std::endl;
      _pending_unmarked_values.clear();
    }
  }
}
}
}
