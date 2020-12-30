#include "logging.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/none.hpp>

#include <logging/format.hpp>

#include <stdexcept>

namespace fs = boost::filesystem;

namespace gitfan {
namespace logging
{
  void createParent(const boost::optional<fs::path>& optionalPath)
  {
    if (!optionalPath) { return; }
    const auto path = optionalPath->parent_path();
    if (!path.empty())
    {
      fs::create_directories(path);
    }
  }

  Log::Log() : Log("not_specified", LoggingOptions())
  {
    std::cout << "WARNING: Creating log with default configuration. Consider "
                 "invoking a parametrised version of Log::instance()."
              << std::endl;
  }

  Log::Log(const std::string& workerName, const LoggingOptions& options)
    : _workerName(workerName)
    , _performanceDumpPath(options.logsDirectory ?
        (*(options.logsDirectory) / "performance_dumps" / workerName) :
        boost::optional<boost::filesystem::path>(boost::none))
    , _counterDumpPath(options.logsDirectory ?
        (*(options.logsDirectory) / "counter_dumps" / workerName) :
        boost::optional<boost::filesystem::path>(boost::none))
    , _logPath(options.logsDirectory ?
        (*(options.logsDirectory) / workerName) :
        boost::optional<boost::filesystem::path>(boost::none))
  {
    createParent(_performanceDumpPath);
    createParent(_counterDumpPath);
    createParent(_logPath);
  }

  Log::~Log()
  {
    try
    {
      dumpCounterStates();
      if (!_counters.empty())
      {
        log("Final counter states:");
        for (const auto& entry : _counters)
        {
          log(entry.first + ": " + std::to_string(entry.second));
        }
      }
      if (!_lastWill.empty())
      {
        log("Last will: " + _lastWill);
      }
    }
    catch (const std::exception& e)
    {
      std::cout << "Exception when deconstructing log for "
                << _workerName << ": " << e.what() << std::endl;
    }
  }

  int& Log::counter(const std::string& tag)
  {
    return _counters[tag];
  }

  void Log::dump(const PerformanceRecord& record) const
  {
    if (!_performanceDumpPath) { return; }

    fs::ofstream o(*_performanceDumpPath,
                   std::ios_base::out | std::ios_base::app);
    o << withNoLineBreaks(record.serialize()) << std::endl;
  }

  void Log::setLastWill(const std::string& lastWill)
  {
    _lastWill = lastWill;
  }

  void Log::dumpCounterStates() const
  {
    if (!_counterDumpPath || _counters.empty()) { return; }

    fs::ofstream o(*_counterDumpPath, std::ios_base::out);
    boost::archive::text_oarchive oa(o);
    oa & _counters;
  }

  std::ostream& Log::getLogStream()
  {
    if(!_logstream)
    {
      if (_logPath)
      {
        _logstream =
          std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
            ( new fs::ofstream(*_logPath,
                               std::ios_base::out | std::ios_base::app)
            , [](std::ostream* p) { delete p; }
            );
      }
      else
      {
        _logstream =
          std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
            (&std::cout, [](std::ostream* p) {});
      }
    }
    return *_logstream;
  }
}
}
