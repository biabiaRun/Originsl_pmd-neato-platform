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

# The version numbering
set (ROYALE_VERSION_MAJOR 3 CACHE STRING "Royale major version" FORCE)
set (ROYALE_VERSION_MINOR 24 CACHE STRING "Royale minor version" FORCE)
set (ROYALE_VERSION_PATCH 0 CACHE STRING "Royale patch version" FORCE)
set (ROYALE_VERSION_BUILD 0 CACHE STRING "Royale build version" FORCE)

# SHA1 "walnuts are great"
set (ROYALE_ACCESS_CODE_LEVEL2 "d79dab562f13ef8373e906d919aec323a2857388" CACHE STRING "Access code for level 2" FORCE)
# SHA1 "we are heroes"
set (ROYALE_ACCESS_CODE_LEVEL3 "c715e2ca31e816b1ef17ba487e2a5e9efc6bbd7b" CACHE STRING "Access code for level 3" FORCE)
# SHA1 "I've been looking for freedom"
set (ROYALE_ACCESS_CODE_LEVEL4 "1a77786a05cc021099dd4b3610b19317fb12e72f" CACHE STRING "Access code for level 4" FORCE)

add_definitions(-DROYALE_ACCESS_CODE_LEVEL2="${ROYALE_ACCESS_CODE_LEVEL2}")
add_definitions(-DROYALE_ACCESS_CODE_LEVEL3="${ROYALE_ACCESS_CODE_LEVEL3}")
add_definitions(-DROYALE_ACCESS_CODE_LEVEL4="${ROYALE_ACCESS_CODE_LEVEL4}")

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

if($ENV{ROYALE_VERSION_BUILD})
    set (ROYALE_VERSION_BUILD $ENV{ROYALE_VERSION_BUILD} CACHE STRING "" FORCE)
endif()

set (JENKINS_BUILD_TYPE "DEVELOPER" CACHE STRING "Build type")

if(DEFINED ENV{JENKINS_BUILD_TYPE})
    set (JENKINS_BUILD_TYPE $ENV{JENKINS_BUILD_TYPE} CACHE STRING "" FORCE)
endif()

if(DEFINED ENV{ROYALE_LICENSE_FILE})
    set (ROYALE_LICENSE_FILE $ENV{ROYALE_LICENSE_FILE} CACHE STRING "Custom License")
else()
    if(${ROYALE_PLAYBACK_ONLY})
        set (ROYALE_LICENSE_FILE "doc/licenses/royaleviewer_license.txt" CACHE STRING "Default License")
	else()
        set (ROYALE_LICENSE_FILE "doc/licenses/royale_license.txt" CACHE STRING "Default License")
	endif()
endif()
set (ROYALE_LICENSE_PATH "${ROYALE_SOURCE_DIR}/${ROYALE_LICENSE_FILE}" CACHE STRING "License Path")

if(DEFINED ENV{ROYALE_CUSTOMER_SUFFIX})
    set (ROYALE_CUSTOMER_SUFFIX $ENV{ROYALE_CUSTOMER_SUFFIX} CACHE STRING "suffix")
endif()

set (EXCEPT_DIRTY_DEV OFF CACHE BOOL "Suppress dirty warnings for developer account")
set (DEV_NAME $ENV{USERNAME} CACHE INTERNAL "Developer user name, to suppress dirty warnings" FORCE)

if (JENKINS_BUILD_TYPE)
    MESSAGE(STATUS "Jenkins build type : ${JENKINS_BUILD_TYPE}")
    if(${JENKINS_BUILD_TYPE} STREQUAL NIGHTLY)
        MESSAGE(STATUS "Configuring nightly build")
        set (ROYALE_VERSION_MAJOR 0 CACHE STRING "Royale major version" FORCE)
        set (ROYALE_VERSION_MINOR 0 CACHE STRING "Royale minor version" FORCE)
        set (ROYALE_VERSION_PATCH 0 CACHE STRING "Royale patch version" FORCE)
        set (ROYALE_NAME "${ROYALE_NAME}_nightly")
        add_definitions(-DROYALE_VERSION_DIRTY)
        if(${EXCEPT_DIRTY_DEV})
          add_definitions(-DDIRTY_EXCEPTION="${DEV_NAME}")
        else()
          add_definitions(-DDIRTY_EXCEPTION="")
        endif()
    elseif(${JENKINS_BUILD_TYPE} STREQUAL RELEASE)
        MESSAGE(STATUS "Configuring release build")
        # do nothing
    else() # DEVELOPER or DIRTY build
        MESSAGE(STATUS "Configuring dirty build")
        set (ROYALE_NAME "${ROYALE_NAME}_dirty")
        add_definitions(-DROYALE_VERSION_DIRTY)
        if(${EXCEPT_DIRTY_DEV})
          add_definitions(-DDIRTY_EXCEPTION="${DEV_NAME}")
        else()
          add_definitions(-DDIRTY_EXCEPTION="")
        endif()
    endif()
endif()

message(STATUS "Royale version : ${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}.${ROYALE_VERSION_BUILD}")
message(STATUS "Royale name    : ${ROYALE_NAME}")

SET(CMAKE_DEBUG_POSTFIX -d)

OPTION(ROYALE_ENABLE_BDD                "Enable BDD catch framework" OFF)
OPTION(ROYALE_ENABLE_TEST_HUB           "Enable building the tool for the test hub" OFF)
OPTION(ROYALE_ENABLE_COV                "Enable test coverage build, only available for debug builds" OFF)
OPTION(ROYALE_ENABLE_DOC                "Build documentation" ON)
OPTION(ROYALE_ENABLE_PANDOC             "Use Pandoc for md documents (Pandoc has to be installed and in the path)" ON)
OPTION(ROYALE_ENABLE_RELEASEANALYZER    "Enable the build of the release anaylzer for unpacking the release and checking integrity" OFF)
OPTION(ROYALE_ENABLE_TOOLS              "Build all internal tools" ON)
OPTION(ROYALE_ENABLE_SAMPLES            "Build all samples" ON)
OPTION(ROYALE_LOGGING_FORCE             "Force the enabling of logging output even with ROYALE_LOGGING set to OFF and in RELEASE mode" OFF)
OPTION(ROYALE_LOGGING                   "Enable logging output globally if DEBUG is on" ON)
OPTION(ROYALE_LOGGING_VERBOSE_BRIDGE    "Enable extra logging in the connectivity layer" OFF)

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
if (DEFINED ENV{QTDIR})
    set (CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

# Beware of reordering these.  The flags from the cmake/* files need
# to come first, otherwise contrib breaks in non-obvious ways.
include(cmake/CMakeLists.txt)

# Define Further options that rely on flags that are set under cmake/CMakeLists
if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
	option(ROYALE_DEVELOPER_EDITION         "Enable developer edition package target" OFF)
endif()

include(cmake/make_version_header.cmake)

# Include Custom.cmake if it is present
if (EXISTS "${ROYALE_SOURCE_DIR}/Custom.cmake")
    message(STATUS "Including custom cmake file : ${ROYALE_SOURCE_DIR}/Custom.cmake")
    include("${ROYALE_SOURCE_DIR}/Custom.cmake")
endif()

# Find the Python3 interpreter before including the contrib directory, otherwise GTest's CMake file
# will find any Python interpreter. Although GTest only needs Python2, roypy wants Python3.
ROYALE_FIND_HOST_PACKAGE(PythonInterp 3.5 REQUIRED)

if (NOT ROYALE_ENABLE_RELEASEANALYZER)
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
endif()

if (ROYALE_ENABLE_TOOLS OR ROYALE_ENABLE_RELEASEANALYZER)
  if(ROYALE_ENABLE_RELEASEANALYZER)
    # Add all 3rd party libraries
    add_subdirectory(contrib)
    add_subdirectory(spectre)
  endif()

  add_subdirectory(tools)
endif()

if (ROYALE_ENABLE_BDD)
  add_subdirectory(test/behavior)
endif()

if (ROYALE_ENABLE_TEST_HUB)
  add_subdirectory(test/testing_hub)
endif()

INSTALL(FILES CHANGELOG.txt DESTINATION ".")

INSTALL_MARKDOWN_FILE("README.md" "Royale ${ROYALE_VERSION}" "Introduction" "${ROYALE_VERSION}" "PUBLIC" ".")
