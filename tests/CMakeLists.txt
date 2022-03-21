include(util-cmake/add_unit_test)

add_subdirectory(singular_data)
set (SINGULAR_TESTDATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/singular_data")

configure_file("conversion.cpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/conversion.cpp" @ONLY)
configure_file("gitfanImpl.cpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/gitfanImpl.cpp" @ONLY)
configure_file ("xpnet_tests.cpp.in"
                "${CMAKE_CURRENT_BINARY_DIR}/xpnet_tests.cpp" @ONLY)
configure_file ("playground.cpp.in"
                "${CMAKE_CURRENT_BINARY_DIR}/playground.cpp" @ONLY)

extended_add_library (NAME Main
  NAMESPACE Test
  SOURCES "main.cpp"
  LIBRARIES gtest
)

add_unit_test(NAME util-gitfan
  SOURCES "simplexfaceiterator.cpp"
          "conestorage.cpp"
          "lru_cache.cpp"
          "${CMAKE_CURRENT_BINARY_DIR}/conversion.cpp"
          "${CMAKE_CURRENT_BINARY_DIR}/gitfanImpl.cpp"
          "util.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
                              "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES Test::Main
            partial_static_${PNET}_wrapper
            GITFAN::storage
            Util::GitfanStatic
            Util::Generic
            gtest
)

add_unit_test(NAME logging
  SOURCES "logging.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
                              "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES Test::Main
            Util::Logging
            Util::Generic
            gtest
            Boost::unit_test_framework
)

add_unit_test(NAME workflows
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/xpnet_tests.cpp"
          "util.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
                              "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES Test::Main
            partial_static_${PNET}_wrapper
            Util::Logging
            Util::GitfanStatic
            Util::Generic
            GITFAN::installation
            GPISpace::execution
            GPISpace::workflow_development
            gtest
            Boost::unit_test_framework
  DEPENDS ${PNET}_and_wrapper-library_target
          full_dim_afaces.pnet_and_wrapper-library_target
          orbit_cones_from_afaces.pnet_and_wrapper-library_target
          traverse_gitfan.pnet_and_wrapper-library_target
          list_manipulation_example.pnet_and_wrapper-library_target
          Config
          singular_testdata
)

macro(add_singular_commands_unit_test name)
  add_unit_test(NAME singular_commands-${name}
    SOURCES "singular_commands/${name}.cpp"
            "util.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
                                "${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARIES Test::Main
              partial_static_${PNET}_wrapper
              Util::GitfanStatic
              Util::Generic
              gtest
              Boost::unit_test_framework
    DEPENDS   singular_testdata
  )
endmacro()

macro(add_singular_commands_configured_unit_test name)
  configure_file("singular_commands/${name}.cpp.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/singular_commands/${name}.cpp"
                 @ONLY)
  add_unit_test(NAME singular_commands-${name}
    SOURCES "${CMAKE_CURRENT_BINARY_DIR}/singular_commands/${name}.cpp"
            "util.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
                                "${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARIES Test::Main
              partial_static_${PNET}_wrapper
              Util::GitfanStatic
              Util::Generic
              gtest
              Boost::unit_test_framework
    DEPENDS   singular_testdata
  )
endmacro()

add_singular_commands_unit_test(call_and_return_result)
add_singular_commands_unit_test(create_symbol_and_check_existance)
add_singular_commands_unit_test(discard_interpreter_output)
add_singular_commands_unit_test(extract_intvec_and_list)
add_singular_commands_unit_test(kill_symbols)
add_singular_commands_unit_test(put_intvec_and_list)
add_singular_commands_configured_unit_test(load_ssi_file)
add_singular_commands_configured_unit_test(write_ssi_file)
add_singular_commands_configured_unit_test(load_library)

extended_add_executable (NAME playground DONT_APPEND_EXE_SUFFIX
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/playground.cpp"
          "util.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
                              "${CMAKE_BINARY_DIR}/src"
  DEPENDS Config
  LIBRARIES Util::Logging
            Util::GitfanStatic
            Util::Generic
            gtest
)

