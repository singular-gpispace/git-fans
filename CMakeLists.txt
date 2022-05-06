cmake_minimum_required (VERSION 3.15)
project (gitfan 
  LANGUAGES C CXX
)

enable_testing()

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set (PNET "gitfan.pnet")
set (WORKFLOW_INSTALLATION "workflow")
set (LIB_INSTALLATION "libexec")

find_package (GPISpace REQUIRED
  COMPONENTS
    DO_NOT_CHECK_GIT_SUBMODULES
)
find_package (util-generic REQUIRED)
find_package (util-rpc REQUIRED)
find_package (util-cmake REQUIRED)
find_package (Boost 1.61.0 REQUIRED
  COMPONENTS
    date_time
    filesystem
    iostreams
    program_options
    regex
    serialization
    system
    thread
    unit_test_framework
)

include (util-cmake/add_macros)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
list (APPEND INSTALL_RPATH_DIRS "libexec/bundle/gpispace/lib")
find_package (Singular REQUIRED)

set (BUNDLE_GPISPACE "libexec/bundle/gpispace")
bundle_GPISpace (DESTINATION "${BUNDLE_GPISPACE}"
  COMPONENTS "runtime"
             "monitoring"
)

install (DIRECTORY "${SINGULAR_HOME}/"
  DESTINATION libexec/bundle/singular
  USE_SOURCE_PERMISSIONS
)

include_directories ("${CMAKE_SOURCE_DIR}/src")
include_directories ("${CMAKE_BINARY_DIR}")

add_subdirectory (singular)
add_subdirectory (googletest)
add_subdirectory (src)
add_subdirectory (workflow)
add_subdirectory (tests)
