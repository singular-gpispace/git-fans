cmake_minimum_required (VERSION 3.13)
project (gitfan LANGUAGES CXX)

include (CTest)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set (INSTALL_DO_NOT_BUNDLE ON CACHE BOOL "")

set (PNET "gitfan.pnet")
set (WORKFLOW_INSTALLATION "workflow")
set (LIB_INSTALLATION "libexec")

add_subdirectory (${GPISPACE_REPO}/cmake cmake)
include (util-cmake/add_macros)
include (util-cmake/add_unit_test)
include (util-cmake/beautify_find_boost)
include (util-cmake/beautify_find_GPISpace)
find_package (GPISpace 21.06 EXACT REQUIRED)
find_boost (1.63 REQUIRED QUIET FROM_GPISPACE_INSTALLATION COMPONENTS
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

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

find_package (Singular REQUIRED)

include_directories ("${CMAKE_SOURCE_DIR}/src")
include_directories ("${CMAKE_BINARY_DIR}")
include_directories ("${GPISPACE_REPO}/src")

add_subdirectory (${GPISPACE_REPO}/src/util-generic src/util-generic)
add_subdirectory (${GPISPACE_REPO}/src/rpc src/rpc)
add_subdirectory (singular)
add_subdirectory (src)
add_subdirectory (workflow)

if (BUILD_TESTING)
  set (INSTALL_GTEST OFF CACHE BOOL "")
  add_subdirectory (googletest)
  add_subdirectory (tests)
endif()
