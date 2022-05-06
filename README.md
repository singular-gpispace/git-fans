# GIT-Fans

This repository accompanies the master's thesis ["Computation of the GIT-fan using a massively parallel implementation"](https://github.com/ChristianReinbold/git-fan_master-thesis/blob/master/build/abschlussarbeit.pdf). It enables the computation of GIT-Fans in massively parallel fashion on scalable hardware systems.

## Abstract

The GIT fan of an algebraic group action on an algebraic variety describes all GIT quotients arising from Mumford's construction in Geometric Invariant Theory. In this thesis we enhance an implementation that computes the GIT fan for toric actions on affine varieties such that it may be executed on scalable hardware systems. For this purpose, we combine the parallelisation framework [GPI-Space](https://github.com/cc-hpc-itwm/gpispace/), developed at the Fraunhofer ITWM, and the computer algebra system [Singular](https://github.com/Singular/Singular/), developed at the Technische Universität Kaiserslautern. In doing so, we are able to compute the Mori chamber decomposition of Mov(Mbar06) (the cone of movable divisor classes of the Deligne-Mumford compactification of the moduli space of 6-pointed stable curves of genus 0) in approximately 21 minutes, utilising 40 Dell PowerEdge M620 Blade Servers with 16 cores each. The previous implementation executed on a single machine terminated after a whole day.

## Installation

The project as well as its dependencies have been built on Ubuntu 20.04 LTS with [GCC](https://gcc.gnu.org/) 9.3.0 and [CMake](https://cmake.org/) 3.16.3. The build process and execution on other setups may or may not succeed.

#### Building Singular

GIT-Fans requires a compatible Singular installation and has been tested with a build of [Singular 4.1.3p2](https://github.com/Singular/Singular/tree/Release-4-1-3p2). Make sure to build Singular from sources according to the Singular installation guide. In our test environment, the following Singular dependencies has been installed as well:

* [4ti2 1.6](http://www.4ti2.de/version_1.6/4ti2-1.6.tar.gz)
* Flint 2.4.4 available at ftp://jim.mathematik.uni-kl.de/pub/Math/Singular/src/4-0-0/flint-2.4.4.tar.gz

#### Building GPI-Space

To build GIT-Fans, the sources as well as a working installation of [GPI-Space v22.03](https://github.com/cc-hpc-itwm/gpispace/tree/v22.03) are required. Check out GPI-Space v22.03 and proceed according to the GPI-Space installation guide. We used the following versions of depending packages:

* [OpenSSL](https://www.openssl.org/) 1.1.1
* [hwloc](https://www.open-mpi.org/projects/hwloc/) 2.1.0
* [Qt5](https://www.qt.io/) 5.12.8
* [chrpath](https://tracker.debian.org/pkg/chrpath) 0.16
* [Boost](https://boost.org) 1.63 (**not 1.61 as in the GPI-Space installation guide**)
* [GPI-2](http://www.gpi-site.com) 1.3.2
* [libssh2](https://www.libssh2.org/) 1.9.0

#### Building GIT-Fans

We assume

- `$GPISPACE_REPO` to be the root of the checked out GPI-Space repository containing the GPI-Space sources
- `$GPISPACE_INSTALL` to be the root of the GPI-Space installation
- `$SINGULAR_INSTALL` to be the root of the Singular installation (the path of the `--prefix` argument when configuring Singular)
- `$FLINT_INSTALL` to be the root of the flint installation (the path of the `--prefix` argument when configuring flint)
- `$ROOT_DIR` to be the root of the checked out GIT-Fans repository
- `$BUILD_DIR` to be a path to a (potentially nonexisting) directory where build files are created
- `$INSTALL_DIR` to be the directory where GIT-Fans is going to be installed

GIT-Fans can be built as follows: First, run cmake to generate the build files.

```bash
mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
      -DCMAKE_BUILD_TYPE=Release \
      -DGSPC_HOME=$GPISPACE_INSTALL \
      -DALLOW_ANY_GPISPACE_VERSION=true \
      -DGPISPACE_REPO=$GPISPACE_REPO \
      -DSINGULAR_HOME=$SINGULAR_INSTALL \
      -DFLINT_HOME=$FLINT_INSTALL \
      $ROOT_DIR
```

Second, navigate to the build directory, then build and install the project.


```bash
cd $BUILD_DIR
make -j$(nproc) install/strip
```

To verify the installation, tests can be run by invoking `make test` in the build directory. Note that the test suite also contains some third party tests since GIT-Fans rebuilds some subdirectories of GPI-Space. In our test environment, the following third-party tests failed (without notable consequences):

* util-generic-hash-combined_hash-performance
* util-generic-hostname
* rpc-performance

However, the following tests provided by GIT-Fans have to be passed:

* util-gitfan
* logging
* workflows
* singular_commands-*

## Using GIT-Fans

GIT-Fans can either be used locally or in union with a job scheduler such as SLURM. A sample script for launching GIT-Fans on supercomputers may look like as follows:


```bash
set -euo pipefail

scriptfile=$(readlink -f "${BASH_SOURCE[0]}")
current_dir="${scriptfile%/*}"

export LD_LIBRARY_PATH=${GPISpace_ROOT}/lib:$LD_LIBRARY_PATH

GITFAN_HOME=${current_dir}/install
GITFAN_REPO=${current_dir}/git-fans
WDIR=/scratch/${USER}/git-fans

NODEFILE="${current_dir}/nodefile"
scontrol show hostnames > $NODEFILE

if [[ -d "$WDIR" ]];
then
  rm -rf ${WDIR}
fi


$GITFAN_HOME/bin/GITFAN \
    --nodefile $NODEFILE \
    --rif-strategy ssh \
    --worker-per-node 16 \
    --job-size 10 \
    --working-directory "$WDIR" \
    --ideal "$GITFAN_REPO/input_files/m6_reduced_ideal" \
    --torus-action "$GITFAN_REPO/input_files/m6_reduced_torus_action" \
    --symmetry-group "$GITFAN_REPO/input_files/m6_reduced_symmetry_group" \
    --simplex-orbit-representatives "$GITFAN_REPO/input_files/m6_reduced_simplex_orbit_representatives" \
    --moving-cone "$WDIR/moving_cone" \
    +CONESTORAGE --subdirectory-name-length 4 --implementation rpc -CONESTORAGE \
    +LOGGING --logs "$WDIR/logs" -LOGGING

echo "Generating statistics"
$GITFAN_HOME/bin/generate_statistics \
    --performance-dumps="$WDIR/logs/performance_dumps" 
    --counter-dumps="$WDIR/logs/counter_dumps" \
    > $WDIR/logs/statistics
```

For further details about usage, please pass the `--help` to the GITFAN executable or result Section 4.6 & 4.7 of the [master's thesis](https://github.com/ChristianReinbold/git-fan_master-thesis/blob/master/build/abschlussarbeit.pdf).
