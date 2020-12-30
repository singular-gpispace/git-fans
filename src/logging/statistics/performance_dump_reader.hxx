#include <logging/performance_record.hpp>
#include <logging/format.hpp>

#include <boost/format.hpp>

#include <fstream>
#include <cerrno>
#include <cmath>


namespace gitfan {
namespace logging {
namespace statistics
{
  template <typename ArithType>
  PerformanceDumpReader<ArithType>::PerformanceDumpReader
    (bool outputStatisticsPerWorker)
    : _outputStatisticsPerWorker(outputStatisticsPerWorker)
  { }

  template <typename ArithType>
  void PerformanceDumpReader<ArithType>::readDump(boost::filesystem::path dump)
  {
    std::ifstream ifs (dump.string());
    if (!ifs)
     {
     throw std::runtime_error
        ( ( boost::format("Could not open file '%1%' for reading: %2%")
          % dump.string()
          % strerror (errno)
          ).str()
        );
    }

    Distribution<ArithType> dumpDistribution;
    for (std::string line; std::getline (ifs, line); )
    {
      auto record =PerformanceRecord::deserialize(withLineBreaks(line));
      auto spanAsIntegral = record.span<std::chrono::milliseconds>().count();
      auto& tagDistribution =
        _distributionsPerTag.emplace ( std::piecewise_construct
                                     , std::forward_as_tuple(record.tag())
                                     , std::forward_as_tuple()
                                     )
                                     .first->second;
      tagDistribution.add(spanAsIntegral);
      dumpDistribution.add(spanAsIntegral);
      _total += spanAsIntegral;
    }
    _distributionsPerDump.emplace(dump.string(), dumpDistribution);
  }

  template <typename ArithType>
  void PerformanceDumpReader<ArithType>::_printDistribution
        ( std::ostream& o
        , const std::string& title
        , const Distribution<ArithType>& d
        ) const
      {
        o << title << ": ("
          << boost::format("%1%")
             % (d.sum() * 100 / static_cast<double>(_total))
          << "\% of total, "
          << d.count() << " records)"
          << std::endl
          << "  Min: " << d.min() << " ms" << " / "
          << "Max: " << d.max() << " ms"
          << std::endl
          << "  Mean: " << d.mean() << " ms" << " / "
          << "Std. deviation: " << sqrt(d.variance()) << " ms";
      }

  template <typename T>
  std::ostream& operator<<(std::ostream& o, const PerformanceDumpReader<T>& r)
  {
    o << "Total: " << r._total << " ms";
    for (const auto& entry : r._distributionsPerTag)
    {
      o << std::endl << std::endl;
      r._printDistribution(o, entry.first, entry.second);
    }
    if (r._outputStatisticsPerWorker)
    {
      for (const auto& entry : r._distributionsPerDump)
      {
        o << std::endl << std::endl;
        r._printDistribution(o, entry.first, entry.second);
      }
    }
    return o;
  }
}
}
}
