SET(Python_ADDITIONAL_VERSIONS "3.5;3.6")
cmake_policy(SET CMP0040 NEW)
cmake_policy(SET CMP0046 NEW)
ROYALE_FIND_HOST_PACKAGE(PythonInterp 3.5 QUIET)
ROYALE_FIND_HOST_PACKAGE(PythonLibs 3.5 QUIET)
include(CMakeParseArguments)

set (ZWETSCHGE_TOOL_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/export_calib.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/lena/writer.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/royale/uuidlike_identifier.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/util/import_device_data.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/util/M2453_B11_utils.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/zwetschge/data_types.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/zwetschge/validator.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool/zwetschge/writer.py"
    )

# timestamp so that all .zwetschge files are regenerated when the tool is edited
add_custom_command(OUTPUT "tool_py_files.stamp"
    COMMAND ${CMAKE_COMMAND} -E touch "tool_py_files.stamp"
    DEPENDS "${ZWETSCHGE_TOOL_SOURCES}"
    )

# Dependency target so that all .zwetschge files are regenerated when the tool is edited
add_custom_target(zwetschgeTool
    DEPENDS "tool_py_files.stamp" "${ZWETSCHGE_TOOL_SOURCES}"
    )

set_property(TARGET zwetschgeTool
    PROPERTY TOOL_MAIN_PY "${CMAKE_CURRENT_SOURCE_DIR}/zwetschge_tool.py"
    )

set_property(TARGET zwetschgeTool
    PROPERTY TOOL_ALL_PY "${ZWETSCHGE_TOOL_SOURCES}"
    )

# Commands for running the ZwetschgeTool
# OUTPUT should be the name of the file to create
# MAIN_DEPENDENCY is the file to pass as ZwetschgeTool's --device argument
# DEPENDS should contain any files that are included from the MAIN_DEPENDENCY
function (generate_zwetschge_file TARGET)
    cmake_policy(SET CMP0046 NEW)
    set (options "GENERATE_LENA")
    set (oneValueArgs "OUTPUT;MAIN_DEPENDENCY")
    set (multiValuesArgs "DEPENDS")
    cmake_parse_arguments (gzf "${options}" "${oneValueArgs}" "${multiValuesArgs}" ${ARGN})
    if (gzf_UNPARSED_ARGS)
        message (FATAL_ERROR "generate_zwetschge_file called with unknown arguments \"${gzf_UNPARSED_ARGS}\"")
    endif ()
    if (NOT DEFINED gzf_OUTPUT)
        message (FATAL_ERROR "generate_zwetschge_file called without OUTPUT argument")
    endif ()
    if (NOT DEFINED gzf_MAIN_DEPENDENCY)
        message (FATAL_ERROR "generate_zwetschge_file called without MAIN_DEPENDENCY argument")
    endif ()
    set (ADDITIONAL_ARGS "")
    if (gzf_GENERATE_LENA)
        set (ADDITIONAL_ARGS "--lena")
    endif()
	get_filename_component(OUTNAME "${gzf_OUTPUT}" NAME)
	get_filename_component(OUTDIR "${gzf_OUTPUT}" DIRECTORY)
    add_custom_command(OUTPUT "${OUTNAME}"
        COMMAND "${PYTHON_EXECUTABLE}" "-B" "$<TARGET_PROPERTY:zwetschgeTool,TOOL_MAIN_PY>" ${ADDITIONAL_ARGS} "--device" "${gzf_MAIN_DEPENDENCY}" "--outfile" "${OUTNAME}"
        MAIN_DEPENDENCY ${gzf_MAIN_DEPENDENCY}
        DEPENDS zwetschgeTool "${gzf_DEPENDS}" "$<TARGET_PROPERTY:zwetschgeTool,TOOL_ALL_PY>"
    )
    add_custom_target("${TARGET}"
        DEPENDS "${OUTNAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${OUTNAME}" "${OUTDIR}/${OUTNAME}"
	)
    add_dependencies("${TARGET}" zwetschgeTool)
endfunction (generate_zwetschge_file)
