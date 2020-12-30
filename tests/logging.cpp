#include <gtest/gtest.h>

#include <logging/logging.hpp>
#include <logging/scope_profiler.hpp>
#include <logging/format.hpp>
#include <logging/statistics/counter_dump_reader.hpp>
#include <logging/statistics/distribution.hpp>
#include <logging/statistics/performance_dump_reader.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/split.hpp>

#include <temporaryUniquePath.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gitfan {
namespace logging {
namespace testing
{
  namespace fs = boost::filesystem;

  class LoggingTest : public ::testing::Test
  {
    private:
      const gitfan::testing::util::TemporaryUniquePath _logsDirectory =
        gitfan::testing::util::TemporaryUniquePath("test-logs-directory");

    public:
      LoggingOptions options;
      const std::string worker =
        fhg::util::testing::random_identifier().substr(0,100);

      LoggingTest()
      {
        std::vector<std::string> params {"--logs", _logsDirectory.string()};
        options = LoggingOptions(params);
      }

      const fs::path logPath()
      {
        return _logsDirectory;
      }
  };

  TEST_F(LoggingTest, logCanLogWithStreamOperatorAndLogFunction)
  {
    const std::string message = fhg::util::testing::random_string();
    {
      Log log(worker, options);
      log << message;
      log.log(message);
    }
    const std::string actual = fhg::util::read_file(logPath() / worker);
    EXPECT_EQ(message + message + "\n", actual);
  }

  TEST_F(LoggingTest, logCanDumpLastWill)
  {
    const std::string message = fhg::util::testing::random_string();
    {
      Log log(worker, options);
      log.setLastWill(fhg::util::testing::random_string());
      log.setLastWill(message);
    }
    const std::string actual = fhg::util::read_file(logPath() / worker);
    EXPECT_EQ("Last will: " + message + "\n", actual);
  }

  TEST_F(LoggingTest, canDumpAndReadTwoCounters)
  {
    std::string tag1 = fhg::util::testing::random_identifier();
    std::string tag2 = fhg::util::testing::random_identifier();
    std::set<std::string> expected;
    expected.emplace("Counter sums:");
    expected.emplace(tag1 + ": " + std::to_string(6));
    expected.emplace(tag2 + ": " + std::to_string(-100));
    if(tag1 == tag2)
    {
      tag2 += "a";
    }
    {
      Log log(worker, options);
      log.counter(tag1)++;
      log.counter(tag2) = -100;
      log.counter(tag1) += 5;
    }
    statistics::CounterDumpReader reader;
    reader.readDump(logPath() / "counter_dumps" / worker);
    std::stringstream sstream;
    sstream << reader;
    const std::list<std::string> readLines =
      fhg::util::split<std::string, std::string>(sstream.str(), '\n');
    std::set<std::string> readLinesAsSet(readLines.begin(), readLines.end());
    EXPECT_EQ(expected, readLinesAsSet);
  }

  TEST_F(LoggingTest, canDumpAndReadTwoPerformanceRecordsByScopeProfiler)
  {
    std::string tag = fhg::util::testing::random_string();
    {
      Log log(worker, options);
      {
        ScopeProfiler sp(log, tag);
      }
      {
        ScopeProfiler sp(log, tag);
      }
    }
    statistics::PerformanceDumpReader<> reader;
    reader.readDump(logPath() / "performance_dumps" / worker);
    std::stringstream sstream;
    sstream << reader;
    const std::string s = sstream.str();
    EXPECT_EQ(tag + ": ", s.substr(s.find("\n")+ 2, tag.size() + 2));
    EXPECT_NE(std::string::npos, s.find("2 records"));
  }

  TEST_F(LoggingTest, emptyDistributionThrows)
  {
    statistics::Distribution<int> d;
    EXPECT_EQ(0ul, d.count());
    EXPECT_EQ(0, d.sum());
    ASSERT_THROW(d.min(), std::runtime_error);
    ASSERT_THROW(d.max(), std::runtime_error);
    ASSERT_THROW(d.mean(), std::runtime_error);
    ASSERT_THROW(d.variance(), std::runtime_error);
  }

  TEST_F(LoggingTest, distributionCanComputeStatisticsOfThreeElements)
  {
    statistics::Distribution<int> d;
    d.add(4);
    d.add(4);
    d.add(4);
    d.add(-1);
    d.add(9);
    EXPECT_EQ(5ul, d.count());
    EXPECT_EQ(-1, d.min());
    EXPECT_EQ(9, d.max());
    EXPECT_EQ(20, d.sum());
    EXPECT_EQ(4, d.mean());
    EXPECT_EQ(10, d.variance());
  }

  TEST_F(LoggingTest, canEscapeLineBreaks)
  {
    const auto s = fhg::util::testing::random_string();
    const auto escaped = withNoLineBreaks(s);
    EXPECT_EQ(std::string::npos, escaped.find("\n"));
    EXPECT_EQ(s, withLineBreaks(escaped));
  }
}
}
}
