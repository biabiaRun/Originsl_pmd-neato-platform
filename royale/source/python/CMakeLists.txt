#****************************************************************************
# Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    RETURN()
ENDIF()

ROYALE_FIND_HOST_PACKAGE(SWIG QUIET)

SET(Python_ADDITIONAL_VERSIONS ${ROYALE_PYTHON_VERSION})
FIND_PACKAGE(PythonLibs 3 REQUIRED)

IF (NOT SWIG_FOUND OR
    NOT PYTHONLIBS_FOUND)
    MESSAGE(WARNING "SWIG not found. Python wrapper will not be built!")
    RETURN()
ENDIF()

MESSAGE(STATUS "Python libraries : ${PYTHONLIBS_VERSION_STRING}")

if (POLICY CMP0086)
  # UseSWIG honors SWIG_MODULE_NAME via -module flag
  cmake_policy(SET CMP0086 NEW)
endif()

INCLUDE(${SWIG_USE_FILE})

SET(CMAKE_SWIG_FLAGS "")
SET(CMAKE_SWIG_OUTDIR ${PROJECT_BINARY_DIR}/lib)

OPTION(ROYALE_USE_NUMPY_IN_ROYPY   "Use numpy.i for the Python wrapper" ON)

if (NOT Numpy_FOUND)
    find_package (Numpy)
    if (Numpy_FOUND)
        set_target_properties(${SWIG_MODULE_roypy_REAL_NAME} PROPERTIES SWIG_COMPILE_DEFINITIONS ACTIVATE_NUMPY)
    else()
        if (ROYALE_USE_NUMPY_IN_ROYPY)
            MESSAGE(WARNING "Numpy activated but Python package was not found! Will disable Numpy usage")
            set (ROYALE_USE_NUMPY_IN_ROYPY OFF CACHE BOOL "" FORCE)
        endif()
    endif()
endif()


if (ROYALE_USE_NUMPY_IN_ROYPY)
    message(STATUS "Numpy version : ${Numpy_VERSION}")
    ADD_DEFINITIONS(-DROYALE_ACTIVATE_NUMPY)
    SET(CMAKE_SWIG_FLAGS -DROYALE_ACTIVATE_NUMPY)
    SET(SWIG_INCLUDE_DIRECTORIES ${ROYALE_NUMPY_I_INCLUDE_DIR})
endif ()

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${LIBSPECTRE_INCLUDE_DIRS}
                    ${PYTHON_INCLUDE_DIRS}
                    ${ROYALE_SOURCE_DIR}/contrib
                    ${ROYALE_SOURCE_DIR}/source/core/inc
                    ${ROYALE_SOURCE_DIR}/source/royale/inc
                    ${Numpy_INCLUDE_DIR}
                    ${ROYALE_NUMPY_I_INCLUDE_DIR}
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
    
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/roypy.i"
    DESTINATION "${ROYALE_INSTALL_PYTHON_DIR}/swig")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/roypy.h"
    DESTINATION "${ROYALE_INSTALL_PYTHON_DIR}/swig")
    
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.template"
    DESTINATION "${ROYALE_INSTALL_PYTHON_DIR}/swig"
    RENAME CMakeLists.txt)

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CMAKE_CURRENT_BINARY_DIR}/README.md")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/README.md"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
    
SET(ROYALE_INSTALL_TO_PYTHON_FOLDER ON CACHE BOOL "" FORCE)

add_subdirectory(samples)

MARK_AS_ADVANCED(ROYALE_INSTALL_TO_PYTHON_FOLDER)
