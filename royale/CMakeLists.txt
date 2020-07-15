#------------------------------------------------------------------------------
# Top-level CMake file
# Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0.0)

message(STATUS "")
message(STATUS "-----------------------------------------------------------")
message(STATUS "| Project Royale                                 (c) 2018 |")
message(STATUS "-----------------------------------------------------------")
message(STATUS "")
message(STATUS "CMake version  : ${CMAKE_VERSION}")

set (ROYALE_NAME "royale")
project(${ROYALE_NAME})

SET(ROYALE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "" FORCE)
SET(ROYALE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "" FORCE)

set (JENKINS_BUILD_TYPE "DEVELOPER" CACHE STRING "Build type")

if(DEFINED ENV{JENKINS_BUILD_TYPE} AND (NOT "$ENV{JENKINS_BUILD_TYPE}" STREQUAL ""))
    set (JENKINS_BUILD_TYPE $ENV{JENKINS_BUILD_TYPE} CACHE STRING "" FORCE)
endif()


include("${CMAKE_CURRENT_LIST_DIR}/cmake/version.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/accesscodes.cmake")

# Introduced in CMake 3.11, CMP0072 is about whether to use libGL or libOpenGL and libGLX on systems
# that have both available. As the project's minimum CMake version is 3.0, set this policy to the
# old behavior for now.
if (POLICY CMP0072)
    cmake_policy(SET CMP0072 OLD)
endif()

# Starting with CMake 3.13, UseSWIG generates now standard target names.
if(POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
endif()

# Introduced in CMake 3.13, BundleUtilities cannot be included at configure time.
if(POLICY CMP0080)
    cmake_policy(SET CMP0080 NEW)
endif()

# Introduced in CMake 3.13, option() honors normal variables.
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set (CMAKE_EXPORT_COMPILE_COMMANDS 1)
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED ON)

OPTION(ROYALE_PLAYBACK_ONLY "Enable only playback, no live cameras" OFF)

IF(${ROYALE_PLAYBACK_ONLY})
    ADD_DEFINITIONS(-DROYALE_PLAYBACK_ONLY)
ENDIF()

if(DEFINED ENV{ROYALE_LICENSE_FILE} AND (NOT "$ENV{ROYALE_LICENSE_FILE}" STREQUAL ""))
    set (ROYALE_LICENSE_FILE $ENV{ROYALE_LICENSE_FILE} CACHE STRING "Custom License")
else()
    if(${ROYALE_PLAYBACK_ONLY})
        set (ROYALE_LICENSE_FILE "doc/licenses/royaleviewer_license.txt" CACHE STRING "Default License")
	else()
        set (ROYALE_LICENSE_FILE "doc/licenses/royale_license.txt" CACHE STRING "Default License")
	endif()
endif()
set (ROYALE_LICENSE_PATH "${ROYALE_SOURCE_DIR}/${ROYALE_LICENSE_FILE}" CACHE STRING "License Path")

INSTALL(FILES ${ROYALE_LICENSE_PATH} DESTINATION "." RENAME "royale_license.txt")

add_definitions (-DROYALE_VERSION_MAJOR=${ROYALE_VERSION_MAJOR})
add_definitions (-DROYALE_VERSION_MINOR=${ROYALE_VERSION_MINOR})
add_definitions (-DROYALE_VERSION_PATCH=${ROYALE_VERSION_PATCH})
add_definitions (-DROYALE_VERSION_BUILD=${ROYALE_VERSION_BUILD})

message(STATUS "Royale version : ${ROYALE_VERSION}")
message(STATUS "Royale name    : ${ROYALE_NAME}")
message(STATUS "Royale package name : ${ROYALE_PACKAGE_NAME}")

SET(CMAKE_DEBUG_POSTFIX -d)

OPTION(ROYALE_ENABLE_BDD                "Enable BDD catch framework" OFF)
OPTION(ROYALE_ENABLE_TEST_HUB           "Enable building the tool for the test hub" OFF)
OPTION(ROYALE_ENABLE_COV                "Enable test coverage build, only available for debug builds" OFF)
OPTION(ROYALE_ENABLE_DOC                "Build documentation" OFF)
OPTION(ROYALE_ENABLE_PANDOC             "Use Pandoc for md documents (Pandoc has to be installed and in the path)" ON)
OPTION(ROYALE_ENABLE_TOOLS              "Build all internal tools" ON)
OPTION(ROYALE_ENABLE_SAMPLES            "Build all samples" ON)
OPTION(ROYALE_LOGGING_FORCE             "Force the enabling of logging output even with ROYALE_LOGGING set to OFF and in RELEASE mode" ON)
OPTION(ROYALE_LOGGING                   "Enable logging output globally if DEBUG is on" ON)
OPTION(ROYALE_LOGGING_VERBOSE_BRIDGE    "Enable extra logging in the connectivity layer" ON)
OPTION(ROYALE_SAVE_RAW_BIN_FILES        "Enable saving the raw data to local disk" OFF)
OPTION(ROYALE_ENABLE_TIME_MEASUREMENTS  "Enables time measurements" OFF)
option(ROYALE_ZIP_PACKAGE               "Use zip packaging instead of an installer" $ENV{ROYALE_ZIP_PACKAGE})

IF(${ROYALE_ENABLE_TIME_MEASUREMENTS})
    ADD_DEFINITIONS(-DROYALE_ENABLE_TIME_MEASUREMENTS)
ENDIF()

SET(ROYALE_PROCESSING_IMPLEMENTATION    "SPECTRE"  CACHE STRING "Switch between Spectre and simple processing. Simple processing does not use any calibration information!" )
SET_PROPERTY(CACHE ROYALE_PROCESSING_IMPLEMENTATION PROPERTY STRINGS "SPECTRE;SIMPLE" )

IF ( "${ROYALE_PROCESSING_IMPLEMENTATION}" STREQUAL "SPECTRE" )
    SET( ROYALE_USE_SPECTRE ON CACHE INTERNAL "" )
    ADD_DEFINITIONS(-DUSE_SPECTRE)
    MESSAGE(STATUS "Use Spectre : True")
ELSEIF ( "${ROYALE_PROCESSING_IMPLEMENTATION}" STREQUAL "SIMPLE" )
    SET( ROYALE_USE_SPECTRE OFF CACHE INTERNAL "" )
    REMOVE_DEFINITIONS(-DUSE_SPECTRE)
    MESSAGE(STATUS "Use Spectre : False")
ELSE()
    SET( ROYALE_USE_SPECTRE OFF CACHE INTERNAL "" )
    MESSAGE(FATAL_ERROR "Unsupported processing implementation")
ENDIF()

# Testing
enable_testing()

# On systems that need it, find QT based on the QTDIR environment variable
if (DEFINED ENV{QTDIR} AND (NOT "$ENV{QTDIR}" STREQUAL ""))
    set (CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

# Beware of reordering these.  The flags from the cmake/* files need
# to come first, otherwise contrib breaks in non-obvious ways.
include(cmake/CMakeLists.txt)

# Define Further options that rely on flags that are set under cmake/CMakeLists
if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
	option(ROYALE_DEVELOPER_EDITION               "Enable developer edition package target" OFF)
	option(ROYALE_LOGGING_WIN_DEBUG_STRING_LOG    "Use Visual Studio Debug String logging as default log backend" OFF)
endif()

if((${ROYALE_TARGET_PLATFORM} STREQUAL LINUX) OR (ROYALE_TARGET_PLATFORM STREQUAL "APPLE"))
	option(ROYALE_LOGGING_SYSLOG    "Use syslog as default log backend" OFF)
endif()

if(${ROYALE_TARGET_PLATFORM} STREQUAL LINUX)
    option(ROYALE_DEB_PACKAGING     "Enables debian packaging" OFF)
endif()

include(cmake/make_version_header.cmake)

# Include Custom.cmake if it is present
if (EXISTS "${ROYALE_SOURCE_DIR}/Custom.cmake")
    message(STATUS "Including custom cmake file : ${ROYALE_SOURCE_DIR}/Custom.cmake")
    include("${ROYALE_SOURCE_DIR}/Custom.cmake")
endif()

if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    set(ROYALE_PYTHON_VERSION "3.7" CACHE INTERNAL "" FORCE)
else()    
    set(ROYALE_PYTHON_VERSION "3.5;3.6;3.7" CACHE INTERNAL "" FORCE)
endif()

# Find the Python3 interpreter before including the contrib directory, otherwise GTest's CMake file
# will find any Python interpreter. Although GTest only needs Python2, roypy wants Python3.
SET(Python_ADDITIONAL_VERSIONS ${ROYALE_PYTHON_VERSION})
ROYALE_FIND_HOST_PACKAGE(PythonInterp 3 REQUIRED)

if(ROYALE_ENABLE_COV)
  OPTION(ROYALE_COVERAGE_GCOVR "If test coverage build is enabled, by default a lcov code coverage report will be produced, setting this flag to ON a gcovr code coverage report will be created" OFF)

  include(${CMAKE_CURRENT_LIST_DIR}/contrib/codeCoverage/CodeCoverage.cmake)
  APPEND_COVERAGE_COMPILER_FLAGS()
endif()

# Add relevant drivers
add_subdirectory(drivers)

# Add all 3rd party libraries
add_subdirectory(contrib)

add_subdirectory(spectre)
  
# Add all samples
add_subdirectory(samples)

# Add all source code
add_subdirectory(source)
  
# Add the documentation and licenses
add_subdirectory(doc)

if (ROYALE_ENABLE_TOOLS)
  add_subdirectory(tools)
endif()

if (ROYALE_ENABLE_BDD)
  add_subdirectory(test/behavior)
endif()

if (ROYALE_ENABLE_TEST_HUB)
  add_subdirectory(test/testing_hub)
endif()

if (ROYALE_DEB_PACKAGING)
    INSTALL(FILES ThirdPartySoftware.txt DESTINATION "share/doc/royale")
    install(FILES CHANGELOG.txt DESTINATION "share/doc/royale")
    install_markdown_file("README.md" "Royale ${ROYALE_VERSION}" "Introduction" "${ROYALE_VERSION}" "PUBLIC" "share/doc/royale")
else()
    INSTALL(FILES ThirdPartySoftware.txt DESTINATION ".")
    INSTALL(FILES CHANGELOG.txt DESTINATION ".")
    INSTALL_MARKDOWN_FILE("README.md" "Royale ${ROYALE_VERSION}" "Introduction" "${ROYALE_VERSION}" "PUBLIC" ".")
endif()
