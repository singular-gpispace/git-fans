#pragma once

#include <util-gitfan/type_aliases.hpp>
#include <Singular/libsingular.h>

#include <boost/filesystem.hpp>

#include <vector>

namespace singular {
namespace gitfan
{
  struct Symbols
  {
    public:
      static const std::string& IDEAL();
      static const std::string& TORUS_ACTION();
      static const std::string& ORBIT_CONES();
      static const std::string& MOVING_CONE();
      static const std::string& SYMMETRY_GROUP();
      static const std::string& SYMMETRY_GROUP_ON_CONES();
      static const std::string& SYMMETRY_GROUP_ON_HASHES();
  };

  void killGlobalSymbols_TESTING_ONLY();

  typedef std::vector<unsigned int> Permutation;
  typedef std::vector<Permutation> GroupAction;

  GroupAction readSymmetryGroupAction
    ( const ::gitfan::pnet_problem_data& data );

  GroupAction readGroupAction
    ( const boost::filesystem::path& ssiFile );

  void writeGroupActionToSSIFile
    ( const GroupAction& action
    , const boost::filesystem::path& target
    );

  void readAndSetDimensionParameters(::gitfan::pnet_problem_data& data);

  void loadMovingCone(const ::gitfan::pnet_problem_data& data);

  void loadSymmetryGroup(const ::gitfan::pnet_problem_data& data);

  void loadSymmetryGroupOnCones(const ::gitfan::pnet_problem_data& data);

  void computeMovingConeAndWriteSSIFile
    ( const ::gitfan::pnet_problem_data& data
    , const boost::filesystem::path& file
    );

  void writeTrivialGroupActionToSSIFile
    ( int cardinality
    , const boost::filesystem::path& file
    );

  ::gitfan::pnet_list findOrbitRepresentatives
    (const ::gitfan::pnet_problem_data& data);

  ::gitfan::pnet_list minimalOrbitRepresentativeOfGroupAction_TESTING_ONLY
    ( const ::gitfan::pnet_list& representative
    , const boost::filesystem::path& groupActionSSIFile
    );

  ::gitfan::pnet_list computeOrbitOfGroupAction_TESTING_ONLY
    ( const ::gitfan::pnet_list& representative
    , const boost::filesystem::path& groupActionSSIFile
    );

  ::gitfan::pnet_list computeOrbitOfSymmetryGroup
    ( const ::gitfan::pnet_problem_data& data
    , const ::gitfan::pnet_list& representative
    );

  std::vector<boost::filesystem::path>
    computeOrbitConeOrbitWithDuplicatesAndWriteSSIFiles
    ( const ::gitfan::pnet_problem_data& data
    , const ::gitfan::pnet_file& representative
    , const boost::filesystem::path& filePrefix
    );

  // Note: ownership of faces is taken by this method.
  ::gitfan::pnet_list computeFullDimAFaces
    ( const ::gitfan::pnet_problem_data& data
    , lists& faces
    );

  void computeOrbitConeAndWriteSSIFile
    ( const ::gitfan::pnet_problem_data& data
    , const ::gitfan::pnet_list& aface
    , const boost::filesystem::path& file
    );

  ::gitfan::pnet_list intersectWithMovingConeAndOverwriteSSIFiles
    ( const ::gitfan::pnet_problem_data& data
    , const ::gitfan::pnet_list& orbitCones
    );

  ::gitfan::pnet_list computeInitialCone
    ( const ::gitfan::pnet_problem_data& data
    , const ::gitfan::pnet_list& orbitCones
    , const boost::filesystem::path& symmetryGroupSSIFile
    );

  ::gitfan::pnet_list_of_sets computeNeighbourHashes
    ( const ::gitfan::pnet_problem_data& data
    , const ::gitfan::pnet_list& orbitCones
    , const boost::filesystem::path& symmetryGroupSSIFile
    , const ::gitfan::pnet_list& conesToExpand
    );
}
}

namespace std
{
  singular::gitfan::Permutation operator*
    ( const singular::gitfan::Permutation& l
    , const singular::gitfan::Permutation& r
    );
}
