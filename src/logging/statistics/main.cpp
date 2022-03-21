#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <logging/statistics/performance_dump_reader.hpp>
#include <logging/statistics/counter_dump_reader.hpp>

#include <util-generic/print_exception.hpp>

#include <iostream>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<po::existing_path> const performanceDumpsOption
      { "performance-dumps"
      , "The directory containing the performance dumps that are included in "
        "the statistics."
      };

    po::option<po::existing_path> const counterDumpsOption
      { "counter-dumps"
      , "The directory containing the counter dumps that are included in the "
        "statistics."
      };

    po::option<bool> const outputWorkerStatisticsOption
      { "output-worker-statistics"
      , "If true, performance statistics are output for each worker."
      , false
      };
  }
}

int main (int argc, char** argv)
try
{
  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("Gitfan-Statistics")
    . add(option::performanceDumpsOption)
    . add(option::counterDumpsOption)
    . add(option::outputWorkerStatisticsOption)
    . store_and_notify(argc, argv)
    );

  namespace fs = boost::filesystem;

  const auto performanceDumpsDirectory =
    option::performanceDumpsOption.get<fs::path>(vm);
  const auto counterDumpsDirectory =
    option::counterDumpsOption.get<fs::path>(vm);

  if (performanceDumpsDirectory)
  {
    gitfan::logging::statistics::PerformanceDumpReader<> reader
      (option::outputWorkerStatisticsOption.get_from(vm));
    for (fs::recursive_directory_iterator file(*performanceDumpsDirectory), end;
         file != end;
         ++file)
    {
      reader.readDump(file->path());
    }
    std::cout << reader << "\n" << std::endl;
  }

  if (counterDumpsDirectory)
  {
    gitfan::logging::statistics::CounterDumpReader reader;
    for (fs::recursive_directory_iterator file(*counterDumpsDirectory), end;
         file != end;
         ++file)
    {
      reader.readDump(file->path());
    }
    std::cout << reader;
  }
  return 0;
}
catch (...)
{
  std::cerr << "EXCEPTION: "
            << fhg::util::current_exception_printer()
            << std::endl
    ;

  return -1;
}

