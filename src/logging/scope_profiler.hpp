#pragma once

#include <logging/logging.hpp>
#include <logging/performance_record.hpp>

namespace gitfan {
namespace logging
{
  class ScopeProfiler
  {
    public:
      ScopeProfiler(const Log& log, const std::string& tag)
        : _log(log)
        , _record(tag)
      { }

      ~ScopeProfiler()
      {
        _record.finish();
        _log.dump(_record);
      }

    private:
      const Log& _log;
      PerformanceRecord _record;
  };
}
}
