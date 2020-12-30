#pragma once

#include <boost/optional.hpp>

#include <chrono>
#include <string>

namespace gitfan {
namespace logging
{
  class PerformanceRecord
  {
    public:
      PerformanceRecord(const std::string& tag)
        : _tag(tag)
        , _start(std::chrono::steady_clock::now())
      { }

      inline const std::string& tag() const
      {
        return _tag;
      }
      template <class ToDuration>
      const ToDuration span() const
      {
        if (!_span)
        {
          throw std::runtime_error("Cannot query an unfinished record.");
        }
        return std::chrono::duration_cast<ToDuration>(*_span);
      }

      void finish();

      std::string serialize() const;
      static PerformanceRecord deserialize(const std::string& serialized);

    private:
      const std::string _tag;
      std::chrono::steady_clock::time_point _start;
      ::boost::optional<std::chrono::steady_clock::duration> _span;
  };
}
}
