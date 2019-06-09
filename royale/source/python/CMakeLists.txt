IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    RETURN()
ENDIF()

ROYALE_FIND_HOST_PACKAGE(SWIG QUIET)

SET(Python_ADDITIONAL_VERSIONS "3.5;3.6")
FIND_PACKAGE(PythonInterp 3 QUIET)
FIND_PACKAGE(PythonLibs 3 QUIET)

IF (NOT SWIG_FOUND OR
    NOT PYTHONLIBS_FOUND)
    MESSAGE("SWIG not found. Python wrapper will not be built!")
    RETURN()
ENDIF()

INCLUDE(${SWIG_USE_FILE})

SET(CMAKE_SWIG_FLAGS "")
SET(CMAKE_SWIG_OUTDIR ${PROJECT_BINARY_DIR}/lib)

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${LIBSPECTRE_INCLUDE_DIRS}
                    ${PYTHON_INCLUDE_DIRS}
                    ${ROYALE_SOURCE_DIR}/src/contrib
                    ${ROYALE_SOURCE_DIR}/source/core/inc
                    ${ROYALE_SOURCE_DIR}/source/royale/inc
                    )
SET_SOURCE_FILES_PROPERTIES(roypy.i PROPERTIES
  CPLUSPLUS ON)

file(GLOB ROYALE_HEADERS
    "${ROYALE_SOURCE_DIR}/source/core/inc/royale/*.hpp"
    "${ROYALE_SOURCE_DIR}/source/royale/inc/royale/*.hpp"
    "${ROYALE_SOURCE_DIR}/source/royale/inc/*.hpp"
)  

# timestamp so that the SWIG file is regenerated when any header is edited
add_custom_command(OUTPUT "roypy.stamp"
    COMMAND ${CMAKE_COMMAND} -E touch "roypy.stamp"
    DEPENDS "${ROYALE_HEADERS}"
    )

add_custom_target(roypyTimestamp ALL
    DEPENDS "roypy.stamp" "${ROYALE_HEADERS}"
    )

SET_TARGET_PROPERTIES(roypyTimestamp
    PROPERTIES
    FOLDER python
)
    
if(${CMAKE_VERSION} VERSION_LESS "3.8.0")   
  SWIG_ADD_MODULE (roypy python
    roypy.i roypy.h "${CMAKE_CURRENT_BINARY_DIR}/roypy.stamp")
else()
  #SWIG_ADD_MODULE was marked as deprecated beginning with CMake 3.8
  SWIG_ADD_LIBRARY(roypy LANGUAGE python
    SOURCES roypy.i roypy.h "${CMAKE_CURRENT_BINARY_DIR}/roypy.stamp")
endif()    

# If we don't have the debug library link against the release
IF(NOT PYTHON_DEBUG_LIBRARIES AND MSVC)
  target_compile_definitions(${SWIG_MODULE_roypy_REAL_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:SWIG_PYTHON_INTERPRETER_NO_DEBUG>)
endif()

SWIG_LINK_LIBRARIES(roypy royale ${PYTHON_LIBRARIES})

SET_TARGET_PROPERTIES(${SWIG_MODULE_roypy_REAL_NAME}
    PROPERTIES
    FOLDER python
    DEBUG_POSTFIX ""
)

if (APPLE) 
   SET_TARGET_PROPERTIES(${SWIG_MODULE_roypy_REAL_NAME} PROPERTIES MACOSX_RPATH TRUE)
   SET_TARGET_PROPERTIES(${SWIG_MODULE_roypy_REAL_NAME} PROPERTIES CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   SET_TARGET_PROPERTIES(${SWIG_MODULE_roypy_REAL_NAME} PROPERTIES INSTALL_RPATH "@loader_path")
else()
   SET_TARGET_PROPERTIES(${SWIG_MODULE_roypy_REAL_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/")
endif()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/test/test_level2.py ${CMAKE_CURRENT_BINARY_DIR}/test/test_level2.py)

add_custom_command(
    TARGET ${SWIG_MODULE_roypy_REAL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SWIG_OUTDIR}/roypy.py
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/roypy.py
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_level1.py
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/test_level1.py
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/test/test_level2.py
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/test_level2.py
    )   

if(NOT ${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_custom_target(hardware_test_roypy
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND "${PYTHON_EXECUTABLE}" ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/test_level1.py
        COMMAND "${PYTHON_EXECUTABLE}" ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/test_level2.py
        DEPENDS ${SWIG_MODULE_roypy_REAL_NAME}
        )
    set_target_properties(hardware_test_roypy PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)

    set_target_properties(hardware_test_roypy
       PROPERTIES
       FOLDER tests/royale
       )

    install(TARGETS ${SWIG_MODULE_roypy_REAL_NAME}
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})

    install(FILES "${CMAKE_SWIG_OUTDIR}/roypy.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/README.md"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/sample_camera_info.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/sample_record_rrf.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/sample_retrieve_data.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/sample_retrieve_depth_image.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})        
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/roypy_sample_utils.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    install(FILES "${ROYALE_SOURCE_DIR}/samples/python/roypy_platform_utils.py"
        DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})

    SET(ROYALE_INSTALL_TO_PYTHON_FOLDER ON CACHE BOOL "" FORCE)
endif()

MARK_AS_ADVANCED(ROYALE_INSTALL_TO_PYTHON_FOLDER)
