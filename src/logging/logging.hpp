#pragma once

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <logging/performance_record.hpp>
#include <logging/logging_options.hpp>

#include <drts/worker/context.hpp>


#include <iostream>
#include <map>
#include <memory>
#include <mutex>


namespace gitfan {
namespace logging
{
  class Log
  {
    private:
      /**
       * Returns a pointer to the current log instance. If no instance is
       * created by now, a log is created and initialised with the given values
       * or default values, depending on createDefaultInstance.
       * As soon as createDefaultInstance is set to false for the first time,
       * a new log will be created, overwriting the default one.
       */
      inline static Log& instance
        ( bool createDefaultInstance
        , const std::string& workerName
        , const std::string& loggingOptions
        )
      {
        static std::unique_ptr<Log> _;
        static std::mutex _mutex;
        if (createDefaultInstance)
        {
          static std::once_flag flag;
          std::call_once(flag, [&]
          {
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_)
            {
              _ = std::make_unique<Log>();
            }
          });
        }
        else
        {
          static std::string instanceOptions;
          static std::once_flag flag;
          std::call_once(flag, [&]
          {
            std::lock_guard<std::mutex> lock(_mutex);
            _ = std::make_unique<Log>
              (workerName, LoggingOptions(loggingOptions));
            instanceOptions = loggingOptions;
          });

          if (loggingOptions != instanceOptions)
          {
            throw std::runtime_error("The log has been initialised already "
                                     "with another set of options");
          }
        }
        return *_;
      }

    public:

      inline static Log& instance()
      {
        return instance(true, "", "");
      }

      inline static Log& instance
        ( const std::string& workerName
        , const std::string& loggingOptions
        )
      {
        return instance(false, workerName, loggingOptions);
      }

      inline static Log& instance
        ( drts::worker::context* context
        , const std::string& loggingOptions
        )
      {
        return instance(false, context->worker_name(), loggingOptions);
      }

      Log();
      Log(const std::string& workerName, const LoggingOptions& options);
      virtual ~Log();

      int& counter(const std::string& tag);
      void dump(const PerformanceRecord& record) const;
      void setLastWill(const std::string& lastWill);

      template <typename T>
      Log& operator<<(const T& value)
      {
        getLogStream() << value;
        return *this;
      }
      template <typename T>
      void log(const T& value)
      {
        getLogStream() << value << std::endl;
      }

    private:
      std::string _workerName;
      ::boost::optional<::boost::filesystem::path> _performanceDumpPath;
      ::boost::optional<::boost::filesystem::path> _counterDumpPath;
      ::boost::optional<::boost::filesystem::path> _logPath;
      std::unique_ptr<std::ostream, std::function<void(std::ostream*)> >
        _logstream;
      std::string _lastWill;
      std::map<std::string, int> _counters;

      std::ostream& getLogStream();

      void dumpCounterStates() const;
  };
}
}

