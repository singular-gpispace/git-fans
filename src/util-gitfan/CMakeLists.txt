include (util-cmake/add_macros)

macro(add_util_gitfan_library suffix type)
  extended_add_library (
    NAME Gitfan${suffix}
    NAMESPACE Util
    TYPE ${type}
    SOURCES "gitfanImpl.cpp"
            "simplexfaceiterator.cpp"
            "ostreamOverloads.cpp"
            "singular_commands.cpp"
            "singular_library_wrapper.cpp"
            "filecontent_operations.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_BINARY_DIR}/src"
                                "${GPISPACE_REPO}/src"
    LIBRARIES Singular
              partial_static_${PNET}_wrapper
              GITFAN::storage
              Util::Logging
              Util::Generic
              GPISpace::workflow_development
              GPISpace::execution
    DEPENDS Config
    POSITION_INDEPENDENT
    INSTALL
    INSTALL_DESTINATION "${LIB_INSTALLATION}"
  )
endmacro()

add_util_gitfan_library("" MODULE)
add_util_gitfan_library("Static" STATIC)
