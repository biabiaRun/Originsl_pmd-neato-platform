#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

cmake_minimum_required(VERSION 2.8.11)

message(STATUS "")
message(STATUS "-----------------------------------------------------------")
message(STATUS "| Project royale platform                        (c) 2018 |")
message(STATUS "-----------------------------------------------------------")
message(STATUS "")

set (PLATFORM_NAME "platform")
set (ROYALE_NAME "royale")

project(${PLATFORM_NAME})

SET(ROYALE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/royale" CACHE INTERNAL "" FORCE)
SET(ROYALE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/royale" CACHE INTERNAL "" FORCE)

# The version numbering
set (PLATFORM_VERSION_MAJOR 0 CACHE STRING "Platform major version" FORCE)
set (PLATFORM_VERSION_MINOR 0 CACHE STRING "Platform minor version" FORCE)
set (PLATFORM_VERSION_PATCH 1 CACHE STRING "Platform patch version" FORCE)
set (PLATFORM_VERSION_BUILD 0 CACHE STRING "Platform build version" FORCE)

set (ROYALE_VERSION_MAJOR 3 CACHE STRING "Royale major version" FORCE)
set (ROYALE_VERSION_MINOR 22 CACHE STRING "Royale minor version" FORCE)
set (ROYALE_VERSION_PATCH 0 CACHE STRING "Royale patch version" FORCE)
set (ROYALE_VERSION_BUILD 0 CACHE STRING "Royale build version" FORCE)

set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

if($ENV{PLATFORM_VERSION_BUILD})
    set (PLATFORM_VERSION_BUILD $ENV{PLATFORM_VERSION_BUILD} CACHE STRING "" FORCE)
endif()

if(DEFINED ENV{JENKINS_PLATFORM_BUILD_TYPE})
    set (JENKINS_PLATFORM_BUILD_TYPE $ENV{JENKINS_PLATFORM_BUILD_TYPE} CACHE STRING "" FORCE)
endif()

if (JENKINS_PLATFORM_BUILD_TYPE)
    MESSAGE(STATUS "Jenkins build type : ${JENKINS_PLATFORM_BUILD_TYPE}")
    if(${JENKINS_PLATFORM_BUILD_TYPE} STREQUAL NIGHTLY)
        set (PLATFORM_VERSION_MAJOR 0 CACHE STRING "Platfrom major version" FORCE)
        set (PLATFORM_VERSION_MINOR 0 CACHE STRING "Platfrom minor version" FORCE)
        set (PLATFORM_VERSION_PATCH 0 CACHE STRING "Platfrom patch version" FORCE)
        set (PLATFORM_NAME "libplatform_nightly")
    endif()
endif()

message(STATUS "Platform version : ${PLATFORM_VERSION_MAJOR}.${PLATFORM_VERSION_MINOR}.${PLATFORM_VERSION_PATCH}.${PLATFORM_VERSION_BUILD}")
message(STATUS "Platform name    : ${PLATFORM_NAME}")

SET(CMAKE_DEBUG_POSTFIX -d)

set (ROYALE_LICENSE_FILE "doc/licenses/royale_license.txt" CACHE STRING "Default License")
set (ROYALE_LICENSE_PATH "${CMAKE_SOURCE_DIR}/royale/${ROYALE_LICENSE_FILE}" CACHE STRING "License Path")

OPTION(ROYALE_ENABLE_PLATFORM_CODE          "Use the platform code" ON)
if (ROYALE_ENABLE_PLATFORM_CODE)
    ADD_DEFINITIONS(-DROYALE_ENABLE_PLATFORM_CODE)
endif()

if(ROYALE_ENABLE_PLATFORM_CODE)
    include(royale/cmake/CMakeLists.txt)

    include(royale/cmake/make_version_header.cmake)

    OPTION(ROYALE_ENABLE_COV       			"Enable test coverage build, only available for debug builds" OFF)
    OPTION(ROYALE_LOGGING          			"Enable logging output globally if DEBUG is on" ON)
    OPTION(ROYALE_ENABLE_EYESAFETY 			"Enable special use cases for testing eye safety" OFF)
    OPTION(ROYALE_ENABLE_PLATFORM_SAMPLE 	"Enable platform playback sample" ON)

    # Use Spectre
    SET( ROYALE_USE_SPECTRE ON CACHE INTERNAL "" )
    ADD_DEFINITIONS(-DUSE_SPECTRE)

    # Testing
    enable_testing()

    SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(royale/contrib/gtest)
    add_subdirectory (royale/spectre)

    include_directories(
        ${CMAKE_SOURCE_DIR}/royale
        ${CMAKE_SOURCE_DIR}/royale/source/components/record/inc
        ${CMAKE_BINARY_DIR}/
        ${ROYALE_BINARY_DIR}/
        ${CMAKE_SOURCE_DIR}/royale/source/core/inc/common
        ${CMAKE_SOURCE_DIR}/royale/source/core/inc
        ${CMAKE_SOURCE_DIR}/royale/source/components/config/inc
        ${CMAKE_SOURCE_DIR}/royale/source/components/imager/inc
        ${CMAKE_SOURCE_DIR}/royale/source/components/factory/inc
        ${CMAKE_SOURCE_DIR}/royale/source/components/processing/inc
        ${PROCESSING_SPECTRE_HEADER_DIR}
        )

    add_subdirectory (royale/source/core)
    add_subdirectory (royale/source/components/imager)
    if (ROYALE_ENABLE_PLATFORM_SAMPLE)
        add_subdirectory (royale/source/components/record)
    endif ()
else()
    OPTION(ROYALE_ENABLE_TOOLS              "Build all internal tools" OFF)
    OPTION(ROYALE_ENABLE_SAMPLES            "Build all samples" OFF)
    set(JENKINS_BUILD_TYPE "RELEASE" CACHE STRING "Build type" FORCE)

    include_directories(
        "${CMAKE_CURRENT_LIST_DIR}/royale/spectre/inc"
        )
    
    add_subdirectory (royale)
endif()    
add_subdirectory (platform)


