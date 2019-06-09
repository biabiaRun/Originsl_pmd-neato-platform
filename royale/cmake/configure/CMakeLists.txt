#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

IF (NOT WIN32 AND NOT CMAKE_BUILD_TYPE)
    MESSAGE( "No build type selected, default to Release" )
    SET( CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE )
ENDIF()

IF (WIN32)
    SET ( ROYALE_USE_SYSTEM_GLUT_LIBRARY "OFF" CACHE STRING "" FORCE )
ELSE()
    SET ( ROYALE_USE_SYSTEM_GLUT_LIBRARY "ON" CACHE STRING "" FORCE )
ENDIF()

# overridden for windows
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" FORCE )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" FORCE )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "" FORCE )

ROYALE_INCLUDE_TARGET_FILE()

IF (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    # This will point to the output folder of the build.
    # Windows/VS has to be treated separately here because it puts the files
    # in a subfolder based on the current configuration (Debug/Release).
    SET( ROYALE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>" CACHE STRING "" FORCE )
ELSE()
    SET( ROYALE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE STRING "" FORCE )
ENDIF()

