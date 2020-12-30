#include <util-gitfan/type_aliases.hpp>
#include <util-gitfan/gitfanInterface.hpp>

#include <pnetc/type/file/op.hpp>

#include <we/type/literal/control.hpp>

#include <algorithm>

// Contains versions of "skip" and "process" used in the
// list_manipulation template.

using pnet_control = we::type::literal::control;

namespace gitfan
{
// list_manipulation_example ------------------------------------------------------------------ //

  inline void list_manipulation_example_init
    ( const pnet_config& config
    , const bool& args
    , bool& skip
    , unsigned long& batch_size
    )
  {
    skip = args;
    batch_size = config.job_size;
  }

  inline pnet_list list_manipulation_example_process
    ( const pnet_list& list
    , unsigned long startIndex
    , unsigned long length
    , const pnet_config& config
    , const bool& args
    )
  {
    pnet_list result;
    const auto start(std::next (list.cbegin(), startIndex));
    const auto end(std::next (start, length));
    std::for_each(start, end, [&](const pnet_value& elem)
    {
      int val = as<int>(elem);
      if (val >= 0)
      {
        result.emplace_back(val * 2);
      }
    });
    return result;
  }

// compute_orbit_cones ------------------------------------------------------ //

  inline void compute_orbit_cones_init
    ( const pnet_config& config
    , const pnet_problem_data& args
    , bool& skip
    , unsigned long& batch_size
    )
  {
    skip = false;
    batch_size = config.job_size;
  }

  inline pnet_list compute_orbit_cones_process
    ( const pnet_list& list
    , unsigned long startIndex
    , unsigned long length
    , const pnet_config& config
    , const pnet_problem_data& args
    )
  {
    auto computeOrbitCone = RESOLVE_INTERFACE_FUNCTION(computeOrbitCone);
    pnet_list result;
    const auto start(std::next (list.cbegin(), startIndex));
    const auto end(std::next (start, length));
    std::for_each(start, end, [&](const pnet_value& aface)
    {
      result.push_back(pnetc::type::file::to_value(computeOrbitCone
        (as<pnet_list>(aface), args, config.working_directory)));
    });
    return result;
  }

// intersect_with_moving_cone ----------------------------------------------- //

  inline void intersect_with_moving_cone_init
    ( const pnet_config& config
    , const pnet_problem_data& args
    , bool& skip
    , unsigned long& batch_size
    )
  {
    skip = !args.moving_cone.valid;
    batch_size = config.job_size;
  }

  inline pnet_list intersect_with_moving_cone_process
    ( const pnet_list& orbitCones
    , unsigned long startIndex
    , unsigned long length
    , const pnet_config& config
    , const pnet_problem_data& args
    )
  {
    auto intersectWithMovingCone =
      RESOLVE_INTERFACE_FUNCTION(intersectWithMovingCone);
    const auto start(std::next (orbitCones.cbegin(), startIndex));
    const auto end(std::next (start, length));
    pnet_list batchOfOrbitCones(start, end);
    return intersectWithMovingCone(args, batchOfOrbitCones);
  }

// expand_orbit_cone_orbits --------------------------------------------------//

  inline void expand_orbit_cone_orbits_init
    ( const pnet_config& config
    , const pnet_problem_data& args
    , bool& skip
    , unsigned long& batch_size
    )
  {
    skip = false;

    // the cost of expanding orbits scales linearly with the group action size.
    // Thus, scale down batch_sizes for large symmetry groups in order to
    // distribute load more evenly.
    batch_size = std::max(1ul, config.job_size / args.symmetry_group_size);
  }

  inline pnet_list expand_orbit_cone_orbits_process
    ( const pnet_list& orbitCones
    , unsigned long startIndex
    , unsigned long length
    , const pnet_config& config
    , const pnet_problem_data& args
    )
  {
    auto expandOrbitConeOrbit =
      RESOLVE_INTERFACE_FUNCTION(expandOrbitConeOrbit);
    pnet_list result;
    auto orbitCone(std::next (orbitCones.cbegin(), startIndex));
    for (int i = startIndex; i < startIndex + length; i++)
    {
      result.push_back(expandOrbitConeOrbit
        ( args
        , config.working_directory
        , pnetc::type::file::from_value(*orbitCone)
        , i
        ));
      ++orbitCone;
    }
    return result;
  }

// keep_unique_orbit_cones ---------------------------------------------------//

  inline void keep_unique_orbit_cones_init
    ( const pnet_config& config
    , const pnet_problem_data& args
    , bool& skip
    , unsigned long& batch_size
    )
  {
    skip = false;
    batch_size = config.job_size;
  }

  inline pnet_list keep_unique_orbit_cones_process
    ( const pnet_list& orbitInfos
    , unsigned long startIndex
    , unsigned long length
    , const pnet_config& config
    , const pnet_problem_data& args
    )
  {
    auto isLastOfAllEqualOrbitConeOrbits =
      RESOLVE_INTERFACE_FUNCTION(isLastOfAllEqualOrbitConeOrbits);
    pnet_list result;
    auto orbitInfo(std::next (orbitInfos.cbegin(), startIndex));
    for (int i = 0; i < length; i++)
    {
      if (isLastOfAllEqualOrbitConeOrbits(args, orbitInfos, orbitInfo))
      {
        result.push_back(*orbitInfo);
      }
      ++orbitInfo;
    }
    return result;
  }

}
