#include <boost/range/adaptor/transformed.hpp>

#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/read_lines.hpp>

#include <util-gitfan/singular_library_wrapper.hpp>
#include <util-gitfan/conversion.hpp>
#include <util-gitfan/type_aliases.hpp>

#include <iostream>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<po::existing_path> const representativeSystemOption
      { "representative-system"
      , "The file containing a representative system for the orbits to compute."
      };

    po::option<po::existing_path> const groupActionOption
      { "group-action"
      , "The group action used to compute the orbits."
      };
  }
}

int main (int argc, char** argv)
try
{
  using namespace gitfan;

  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("expand-orbits")
    . require(option::representativeSystemOption)
    . require(option::groupActionOption)
    . store_and_notify(argc, argv)
    );
  const auto representativeFile =
    option::representativeSystemOption.get_from(vm);
  const auto groupActionFile = option::groupActionOption.get_from(vm);

  unsigned long orbitCount = 0;
  unsigned long coneCount = 0;

  for ( const std::string& hashString
      : fhg::util::read_lines(representativeFile)
      )
  {
    pnet_list orbit = singular::gitfan::computeOrbitOfGroupAction_TESTING_ONLY
      ( conversion::hexStringToList(hashString)
      , groupActionFile
      );

    for ( const auto& cone : orbit
        | boost::adaptors::transformed(&as<pnet_list>)
        )
    {
      coneCount++;
      std::cout << conversion::listToHexString(cone, hashString.size() * 4)
                << "\n";
    }
    orbitCount++;
    std::cout << "\n";
  }
  std::cout << "Orbits: " << orbitCount << "\n";
  std::cout << "Cones: " << coneCount << "\n";

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
