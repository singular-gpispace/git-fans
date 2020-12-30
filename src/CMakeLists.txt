function (determine_git_revision directory output_variable_name)
  find_package (Git REQUIRED)

  set (command "${GIT_EXECUTABLE}" rev-parse HEAD)
  execute_process (COMMAND ${command}
    WORKING_DIRECTORY "${directory}"
    OUTPUT_VARIABLE output OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE error_code)

  if (NOT ${error_code} EQUAL 0)
    string (REPLACE ";" " " command_string "${command}")
    message (FATAL_ERROR "could not discover revision info for '${directory}': "
      "`${command_string}` failed with error code ${error_code}. "
      "output: ${output}")
  endif()

  set (${output_variable_name} "${output}" PARENT_SCOPE)
endfunction()

add_subdirectory (logging)
add_subdirectory (storage)
add_subdirectory (util-gitfan)

configure_file ("config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/config.hpp" @ONLY)
add_custom_target (Config
  ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/config.hpp"
)

determine_git_revision ("${CMAKE_SOURCE_DIR}" PROJECT_REVISION)
configure_file ("project_revision.hpp.in"
                "${CMAKE_CURRENT_BINARY_DIR}/project_revision.hpp" @ONLY)
add_custom_target (ProjectRevision
  ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/project_revision.hpp"
)

configure_file ("installation.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/installation.cpp"
  @ONLY
)
extended_add_library (NAME installation
  NAMESPACE GITFAN
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/installation.cpp"
  LIBRARIES Util::Generic
            GPISpace::execution
)

extended_add_executable (NAME GITFAN DONT_APPEND_EXE_SUFFIX
  SOURCES "bin/gitfan.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES partial_static_${PNET}_wrapper
            GITFAN::installation
            GITFAN::storage
            GPISpace::execution
            GPISpace::workflow_development
            Util::Generic
            Util::Logging
            Util::GitfanStatic
  DEPENDS Config
          ProjectRevision
  INSTALL
)

extended_add_executable (NAME expand_orbits DONT_APPEND_EXE_SUFFIX
  SOURCES "bin/expand_orbits.cpp"
  LIBRARIES Util::Generic
            Util::Logging
            Util::GitfanStatic
  INSTALL
)
