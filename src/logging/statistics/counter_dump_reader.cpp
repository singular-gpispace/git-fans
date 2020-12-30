#include <logging/statistics/counter_dump_reader.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>

#include <boost/format.hpp>

namespace gitfan {
namespace logging {
namespace statistics
{
  void CounterDumpReader::readDump(boost::filesystem::path dump)
  {
    std::map<std::string, int> dumpedCounters;
    {
      std::ifstream ifs(dump.string());
      if (!ifs)
      {
        throw std::runtime_error
          ( ( boost::format("Could not open file '%1%' for reading: %2%")
            % dump.string()
            % strerror (errno)
            ).str()
          );
      }

      boost::archive::text_iarchive ia(ifs);
      ia & dumpedCounters;
    }
    for (const auto& counter : dumpedCounters)
    {
      _counters[counter.first] += counter.second;
    }
  }

  std::ostream& operator<<(std::ostream& o, const CounterDumpReader& r)
  {
    o << "Counter sums:\n";

    for (const auto& counter : r._counters)
    {
      o << counter.first << ": " << counter.second << "\n";
    }
    return o;
  }
}
}
}
