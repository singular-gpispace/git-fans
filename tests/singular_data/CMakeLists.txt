macro(configure_files srcDir destDir)
    set(createdFiles "")
    make_directory(${destDir})
    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    @ONLY)
            list(APPEND createdFiles ${destDir}/${templateFile})
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)

configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command (
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/m5_git_cones"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/generate.sh" ${SINGULAR_HOME}
    WORKING_DIRECTORY ${SINGULAR_TESTDATA_DIR}
    DEPENDS "${createdFiles}"
  )
add_custom_target (singular_testdata
  ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/m5_git_cones"
)
