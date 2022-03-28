#include <installation.hpp>

#include <storage/service_provider.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/boost/program_options/validators/nonempty_file.hpp>

#include <we/type/value/poke.hpp>
#include <we/type/value/show.hpp>

#include <util-gitfan/gitfanInterface.hpp>
#include <util-gitfan/nonexisting_path_or_nonempty_file.hpp>
#include <util-gitfan/type_aliases.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <sstream>

#include <config.hpp>
#include <project_revision.hpp>

#include <logging/command_line_args_printer.hpp>
#include <logging/logging.hpp>
#include <logging/scope_profiler.hpp>
#include <logging/logging_options.hpp>

#include <pnetc/type/problem_data/op.hpp>
#include <pnetc/type/config/op.hpp>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;
    namespace gfo = gitfan::boost::program_options;

    po::option<po::positive_integral<unsigned long>> const
      worker_per_node { "worker-per-node"
                      , "number of workers per node"
                      };
    po::option<po::positive_integral<unsigned long>> const
      job_size { "job-size"
               , "The amount of units (e.g. simplex faces or cones) to process in one job"
               , 100
               };
    po::option<po::nonexisting_path> const working_directory
      { "working-directory"
      , "working directory, will contain temporary data"
      };
    po::option<po::nonempty_file> const ideal
      { "ideal"
      , "path to a file containing the input ideal"
      };
    po::option<po::nonempty_file> const torus_action
      { "torus-action"
      , "path to a file containing the torus action encoded as a grading matrix"
      };
    po::option<po::nonempty_file> const symmetry_group
      { "symmetry-group"
      , "Optional path to a file containing a symmetry group of the torus "
        "action."
      };
    po::option<po::nonempty_file> const simplex_orbit_representatives
      { "simplex-orbit-representatives"
      , "If supplied, the orbits of the simplex faces are constructed from the "
        "given representatives."
      };
    po::option<po::existing_path> const precomputed_orbit_cones
      { "precomputed-orbit-cones"
      , "If supplied, the orbit cones referenced in the given file "
        "- relatively to its parent directory - are used for running the "
        "graph traversal, effectively skipping the first phase of the "
        "algorithm."
      };

    po::option<po::existing_path> const precomputed_symmetry_group_on_hashes
      { "precomputed-symmetry-group-on-hashes"
      , "If a value for precomputed-orbit-cones is provided, this parameter "
        "specifies the symmetry group permuting the precomputed set of "
        "orbit cones. If no value is given, the trivial group is assumed. "
        "Note that the symmetry-group parameter is ignored when "
        "precomputed-orbit-cones is provided."
      };

    po::option<gfo::nonexisting_path_or_nonempty_file> const moving_cone
      { "moving-cone"
      ,  "path to a file containing the precomputed moving cone. If the given "
         "path does not exist, the moving cone is computed and written to the "
         "given path. Omit this option, if the moving cone should not be "
         "considered at all."
      };

    po::option<long> const max_cones
      { "max-cones"
      , "maximal number of expanded git-cones before the algorithm terminates. "
        "Negative numbers indicate that the algorithm should stop after "
        "expanding all git-cones."
      , -1
      };

    po::embedded_command_line const conestorage_option
      { "conestorage"
      , "options for configuring the cone storage"
      , "+CONESTORAGE"
      , "-CONESTORAGE"
      };

    po::embedded_command_line const logging_option
      { "logging"
      , "options used for logging"
      , "+LOGGING"
      , "-LOGGING"
      };
  }
}

int main (int argc, char** argv)
try
{
  gitfan::logging::CommandLineArgsPrinter argsPrinter(argc, argv);
  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("gitfan")
    . add (gspc::options::logging())
    . add (gspc::options::scoped_rifd())
    . add ( fhg::util::boost::program_options::options ("Topology")
          . add (option::job_size)
          . require (option::worker_per_node)
          )
    . add ( fhg::util::boost::program_options::options ("Job")
          . require (option::working_directory)
          . require (option::ideal)
          . require (option::torus_action)
          . add (option::moving_cone)
          . add (option::symmetry_group)
          . add (option::simplex_orbit_representatives)
          . add (option::precomputed_orbit_cones)
          . add (option::precomputed_symmetry_group_on_hashes)
          . add (option::max_cones)
          . embed (option::conestorage_option)
          . embed (option::logging_option)
          )
    . store_and_notify (argc, argv)
    );

  auto workingDirectory = option::working_directory.get_from(vm);
  boost::filesystem::create_directories(workingDirectory);
  argsPrinter.printToFile(workingDirectory / "args");
  {
    boost::filesystem::ofstream o(workingDirectory / "rev");
    o << config::projectRevision;
  }

  std::ostringstream topology_description;
  topology_description
    << "worker:" << option::worker_per_node.get_from (vm) << " "
    << "get_expandable_cones:1";

  gitfan::installation const gitfan_installation
    (vm, fhg::util::executable_path().parent_path().parent_path());
  gspc::installation const gspc_installation
    (gitfan_installation.gspc_installation());

  gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                 , gspc::rifd::hostnames (vm)
                                 , gspc::rifd::port (vm)
                                 , gspc_installation
                                 };

  gspc::scoped_runtime_system const drts
    (vm, gspc_installation, topology_description.str(), rifds.entry_points());

auto const ppp = option::conestorage_option.get_from (vm);
// std::cout << ppp.size() << std::endl;
// std::cout << workingDirectory << std::endl;
auto sym = RESOLVE_INTERFACE_FUNCTION(initStorageOptions) (ppp, workingDirectory);
// std::cout << "nice guys, that worked out okay" << std::endl;
  gitfan::storage::ServiceProvider storageServiceProvider
    (sym
    );

  gitfan::pnet_problem_data data;

  data.ideal.path = option::ideal.get_from (vm).string();
  data.Q.path = option::torus_action.get_from (vm).string();

  data.moving_cone.valid = option::moving_cone.is_contained_in(vm);
  data.moving_cone.path = data.moving_cone.valid
                          ? option::moving_cone.get_from(vm).string()
                          : "";

  data.symmetry_group.valid = option::symmetry_group.is_contained_in(vm);
  data.symmetry_group.path = data.symmetry_group.valid
                             ? option::symmetry_group.get_from(vm).string()
                             : "";

  data.simplex_orbit_representatives.valid =
    option::simplex_orbit_representatives.is_contained_in(vm);
  data.simplex_orbit_representatives.path =
    data.simplex_orbit_representatives.valid
    ? option::simplex_orbit_representatives.get_from(vm).string()
    : "";

  data.precomputed_orbit_cones.valid =
    option::precomputed_orbit_cones.is_contained_in(vm);
  data.precomputed_orbit_cones.path = data.precomputed_orbit_cones.valid
    ? option::precomputed_orbit_cones.get_from(vm).string()
    : "";

  data.precomputed_symmetry_group_on_hashes.valid =
    option::precomputed_symmetry_group_on_hashes.is_contained_in(vm);
  data.precomputed_symmetry_group_on_hashes.path =
    data.precomputed_symmetry_group_on_hashes.valid
    ? option::precomputed_symmetry_group_on_hashes.get_from(vm).string()
    : "";

  gitfan::pnet_config config;
  config.job_size = option::job_size.get_from(vm);
  config.max_job_queue = 3 * rifds.hosts().size() *
                         option::worker_per_node.get_from (vm);
  config.working_directory.path = workingDirectory.string();
  config.max_cones = option::max_cones.get_from(vm);
  config.storage_options = storageServiceProvider.options().serialize();
  config.logging_options =
    gitfan::logging::LoggingOptions(option::logging_option.get_from(vm))
    .serialize();

  std::multimap<std::string, pnet::type::value::value_type> values_on_ports
    ( { {"problem_data", pnetc::type::problem_data::to_value(data)}
      , {"config", pnetc::type::config::to_value(config)}
      }
    );

  gitfan::logging::ScopeProfiler record
    ( gitfan::logging::Log::instance("main", config.logging_options)
    ,"main"
    );
  auto const workflow_result
    ( gspc::client (drts).put_and_run
        (gspc::workflow (gitfan_installation.workflow()), values_on_ports)
    );

  for (auto const& r : workflow_result)
  {
    std::cout << r.first
              << ": "
              << pnet::type::value::show (r.second)
              << '\n'
      ;
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
