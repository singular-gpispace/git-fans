#!/bin/bash
rm -rf sample_cones
rm -rf m5_orbit_cones
rm -rf sample_ssi_files
rm -f m5_full_dim_aface_orbit_representatives
rm -f m5_full_dim_afaces
rm -f m5_git_cone_orbit_representatives
rm -f m5_git_cones
rm -f m5_moving_cone
rm -f m5_symmetry_group_on_hashes
rm -f m5_symmetry_group_on_hashes_orbit1
rm -f m5_symmetry_group_on_hashes_orbit2
rm -f m5_symmetry_group_on_hashes_orbit3
rm -f m5_symmetry_group_on_hashes_orbit4
rm -f m5_trivial_symmetry_group_on_hashes
mkdir sample_cones
mkdir m5_orbit_cones
mkdir sample_ssi_files
$1/bin/Singular < "generation" > /dev/null
