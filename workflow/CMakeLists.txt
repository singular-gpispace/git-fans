macro(copy_petrinet_over_to_build name)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${name}.xpnet" "${CMAKE_CURRENT_BINARY_DIR}/${name}.xpnet" COPYONLY)
endmacro()

copy_petrinet_over_to_build(full_dim_afaces)
copy_petrinet_over_to_build(full_dim_afaces_frame)
copy_petrinet_over_to_build(gitfan)
copy_petrinet_over_to_build(list_manipulation_example_frame)
copy_petrinet_over_to_build(orbit_cones_from_afaces)
copy_petrinet_over_to_build(orbit_cones_from_afaces_frame)
copy_petrinet_over_to_build(traverse_gitfan)
copy_petrinet_over_to_build(traverse_gitfan_frame)
copy_petrinet_over_to_build(types)


macro(specialize_list_manipulation name)
  set(TEMPLATE_ARGS_TYPE "problem_data")
  set(TEMPLATE_PREFIX "${name}")
  configure_file ("list_manipulation.xpnet.in" "${CMAKE_CURRENT_BINARY_DIR}/${name}.xpnet" @ONLY)
endmacro()

set(TEMPLATE_ARGS_TYPE "bool")
set(TEMPLATE_PREFIX "list_manipulation_example")
configure_file ("list_manipulation.xpnet.in" "${CMAKE_CURRENT_BINARY_DIR}/list_manipulation_example.xpnet" @ONLY)

specialize_list_manipulation(compute_orbit_cones)
specialize_list_manipulation(intersect_with_moving_cone)
specialize_list_manipulation(expand_orbit_cone_orbits)
specialize_list_manipulation(keep_unique_orbit_cones)
add_custom_target (TemplatedPetrinets
  ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/list_manipulation_example.xpnet"
          "${CMAKE_CURRENT_BINARY_DIR}/compute_orbit_cones.xpnet"
          "${CMAKE_CURRENT_BINARY_DIR}/intersect_with_moving_cone.xpnet"
          "${CMAKE_CURRENT_BINARY_DIR}/expand_orbit_cone_orbits.xpnet"
          "${CMAKE_CURRENT_BINARY_DIR}/keep_unique_orbit_cones.xpnet"
)

macro(add_petrinet in out)
  set(gen_type_op_sources
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/file/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/problem_data/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/optional_file/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/optional_directory/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/config/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/directory/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/list_range/op.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/type/range/op.cpp"
  )
  add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/op/libgitfan.so"
           ${gen_type_op_sources}
    COMMAND 
        GPISpace::pnetc
            --gen-cxxflags=-std=c++14
            --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}"
            --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}/src"
            --gen-cxxflags=-I"${CMAKE_BINARY_DIR}/src"
            --gen-ldflags="$<TARGET_LINKER_FILE:Util::Logging>"
            --gen-ldflags="$<TARGET_LINKER_FILE:Util::Generic>"
            --gen-ldflags="-lboost_program_options"
            --force-overwrite-file=true
            --input="${CMAKE_CURRENT_BINARY_DIR}/${in}"
            --output="${CMAKE_CURRENT_BINARY_DIR}/${out}"
            --path-to-cpp="${CMAKE_CURRENT_BINARY_DIR}/gen_${out}"
            --Wbackup-file=OFF
            --Woverwrite-file=OFF
            && $(MAKE) -C "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/gitfan.xpnet"
            "${CMAKE_CURRENT_BINARY_DIR}/full_dim_afaces.xpnet"
            "${CMAKE_CURRENT_BINARY_DIR}/orbit_cones_from_afaces.xpnet"
            "${CMAKE_CURRENT_BINARY_DIR}/traverse_gitfan.xpnet"
            "${CMAKE_CURRENT_BINARY_DIR}/types.xpnet"
            "${CMAKE_CURRENT_SOURCE_DIR}/list_manipulation_function_container.hpp"
            "${CMAKE_SOURCE_DIR}/src/util-gitfan/gitfanInterface.hpp"
            "${CMAKE_SOURCE_DIR}/src/logging/logging.hpp"
            "${CMAKE_SOURCE_DIR}/src/logging/logging_options.hpp"
            "${CMAKE_SOURCE_DIR}/src/logging/performance_record.hpp"
            Config
            TemplatedPetrinets
            Util::Generic
            Util::Logging
  )
  add_custom_target (${out}_and_wrapper-library_target
    ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/gen_${out}/pnetc/op/libgitfan.so"
            ${gen_type_op_sources}
  )

  extended_add_library (
    NAME partial_static_${out}_wrapper
    TYPE STATIC
    SOURCES ${gen_type_op_sources}
    INCLUDE_DIRECTORIES PUBLIC "${CMAKE_BINARY_DIR}/workflow/gen_${out}"
    LIBRARIES GPISpace::workflow_development
              GPISpace::execution
    DEPENDS ${out}_and_wrapper-library_target
    POSITION_INDEPENDENT
  )
endmacro()

add_petrinet(gitfan.xpnet ${PNET})
add_petrinet(full_dim_afaces_frame.xpnet full_dim_afaces.pnet)
add_petrinet(orbit_cones_from_afaces_frame.xpnet orbit_cones_from_afaces.pnet)
add_petrinet(traverse_gitfan_frame.xpnet traverse_gitfan.pnet)
add_petrinet(list_manipulation_example_frame.xpnet list_manipulation_example.pnet)

install (FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PNET}"
  DESTINATION "${WORKFLOW_INSTALLATION}"
)
install (FILES
  "${CMAKE_CURRENT_BINARY_DIR}/gen_${PNET}/pnetc/op/libgitfan.so"
  DESTINATION "${LIB_INSTALLATION}"
)
