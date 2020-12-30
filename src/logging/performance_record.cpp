#include "performance_record.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/optional.hpp>

#include <util-generic/serialization/std/chrono.hpp>

#include <stdexcept>
#include <sstream>

namespace gitfan {
namespace logging
{
  void PerformanceRecord::finish()
  {
    if(_span)
    {
      throw std::runtime_error("Cannot end a finished record.");
    }
    _span = std::chrono::steady_clock::now() - _start;
  }

  std::string PerformanceRecord::serialize() const
  {
    std::ostringstream oss;
    boost::archive::text_oarchive oa(oss);
    oa & _tag;
    oa & _start;
    oa & _span;
    return oss.str();
  }

  PerformanceRecord PerformanceRecord::deserialize
    (const std::string& serialized)
  {
    std::string tag;
    std::chrono::steady_clock::time_point start;
    boost::optional<std::chrono::steady_clock::duration> span;

    std::istringstream iss(serialized);
    boost::archive::text_iarchive ia (iss);
    ia & tag;
    ia & start;
    ia & span;

    PerformanceRecord record(tag);
    record._start = start;
    record._span = span;
    return record;
  }
}
}
