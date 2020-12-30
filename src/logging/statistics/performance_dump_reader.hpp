#pragma once

#include <boost/filesystem.hpp>

#include <logging/statistics/distribution.hpp>

#include <map>
#include <string>
#include <iostream>
#include <chrono>

namespace gitfan {
namespace logging {
namespace statistics
{
  template <typename ArithType = std::chrono::milliseconds::rep>
  class PerformanceDumpReader
  {
    public:
      PerformanceDumpReader(bool outputStatisticsPerWorker = false);
      void readDump(boost::filesystem::path dump);

      template<typename T>
      friend std::ostream& operator<<(std::ostream& o,
                                      const PerformanceDumpReader<T>& r);

    private:
      std::map<std::string, Distribution<ArithType> > _distributionsPerTag;
      std::map<std::string, Distribution<ArithType> > _distributionsPerDump;
      bool _outputStatisticsPerWorker;
      ArithType _total = 0;

      void _printDistribution
        ( std::ostream& o
        , const std::string& title
        , const Distribution<ArithType>& d
        ) const;
  };

  template <typename T>
  std::ostream& operator<<(std::ostream& o, const PerformanceDumpReader<T>& r);
}
}
}

#include <logging/statistics/performance_dump_reader.hxx>
