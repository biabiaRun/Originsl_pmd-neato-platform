#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

if (CyAPI_FOUND)
    return()
endif()

include (FindPackageHandleStandardArgs)

if (NOT DEFINED ENV{FX3_INSTALL_PATH})
    # if REQUIRED was set then this will exit with a FATAL_ERROR, otherwise we return
    find_package_handle_standard_args (CyAPI "Not searching for CyAPI libraries, because FX3_INSTALL_PATH is not in the environment" ENV{FX3_INSTALL_PATH} )
    return()
endif()

FIND_PATH( CyAPI_INCLUDE_DIR CyAPI.h
          HINTS "$ENV{FX3_INSTALL_PATH}/library/cpp/inc"
          PATHS "$ENV{FX3_INSTALL_PATH}/library/cpp/inc"
          )

if (CyAPI_FIND_VERSION)
    FILE(READ "${CyAPI_INCLUDE_DIR}/VersionNo.h" CyAPIVersionHeader)
    STRING(REGEX MATCH "\#define FILEVER([ ]+)([0-9]+),([0-9]+),([0-9]+),([0-9]+)" CyAPIFileVersionDefine ${CyAPIVersionHeader})
    STRING(REGEX MATCH "([0-9]+),([0-9]+),([0-9]+),([0-9]+)" CyAPIVersion ${CyAPIFileVersionDefine})
    STRING(REPLACE "," "." CyAPIVersion ${CyAPIVersion})
    
    if (NOT CyAPI_FIND_VERSION VERSION_EQUAL CyAPIVersion)
        message (WARNING "Note: CyAPI version numbers are different to the SDK versions, SDK 1.3.3 has CyAPI 1.2.1")
        message (WARNING "Note: CyAPI version numbers are different to the SDK versions, SDK 1.3.4 has CyAPI 1.2.3")
        message (FATAL_ERROR "CyAPI Version does not match! Requested version ${CyAPI_FIND_VERSION}, but found version ${CyAPIVersion}!")
    endif()
    
    ADD_DEFINITIONS(-DCYAPIVERSION=${CyAPIVersion})
endif ()

if (${ARCHITECTURE_BITNESS} STREQUAL 32)
    FIND_LIBRARY( CyAPI_LIBRARY
                 NAMES CyAPI.lib
                 HINTS "$ENV{FX3_INSTALL_PATH}/library/cpp/lib/x86"
                 PATHS "$ENV{FX3_INSTALL_PATH}/library/cpp/lib/x86"
                 PATH_SUFFIXES ""
                 NO_DEFAULT_PATH
                 )
elseif (${ARCHITECTURE_BITNESS} STREQUAL 64)
    FIND_LIBRARY( CyAPI_LIBRARY
                 NAMES CyAPI.lib
                 HINTS "$ENV{FX3_INSTALL_PATH}/library/cpp/lib/x64"
                 PATHS "$ENV{FX3_INSTALL_PATH}/library/cpp/lib/x64"
                 PATH_SUFFIXES ""
                 NO_DEFAULT_PATH
                 )

else ()
    message (FATAL_ERROR "Unknown / unsupported ARCHITECTURE_BITNESS")
endif()

find_package_handle_standard_args (CyAPI
   FOUND_VAR CyAPI_FOUND
   REQUIRED_VARS CyAPI_LIBRARY CyAPI_INCLUDE_DIR
   VERSION_VAR CyAPIVersion
   HANDLE_COMPONENTS)

if (CyAPI_LIBRARY AND CyAPI_INCLUDE_DIR)
    add_library(CyAPI STATIC IMPORTED GLOBAL)
    set_property(TARGET CyAPI
        APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${CyAPI_INCLUDE_DIR}"
        )
    set_target_properties(CyAPI PROPERTIES
        IMPORTED_LOCATION "${CyAPI_LIBRARY}"
        )
endif()
