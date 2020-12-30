#pragma once

#include <boost/filesystem.hpp>

#include <iostream>
#include <map>

namespace gitfan {
namespace logging {
namespace statistics
{
  class CounterDumpReader
  {
  public:
    void readDump(boost::filesystem::path dump);

    friend std::ostream& operator<<(std::ostream& o,
                                    const CounterDumpReader& r);
  private:
    std::map<std::string, int> _counters;
  };

  std::ostream& operator<<(std::ostream& o, const CounterDumpReader& r);
}
}
}
