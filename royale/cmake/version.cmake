#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

#TODO : Come up with a smarter solution to make a new version!!!

# The version numbering
set (ROYALE_VERSION_MAJOR 3 CACHE STRING "Royale major version" FORCE)
set (ROYALE_VERSION_MINOR 31 CACHE STRING "Royale minor version" FORCE)
set (ROYALE_VERSION_PATCH 0 CACHE STRING "Royale patch version" FORCE)
set (ROYALE_VERSION_BUILD 0 CACHE STRING "Royale build version" FORCE)

string(TIMESTAMP CURRENT_DATE %Y%m%d)

if(DEFINED ENV{ROYALE_VERSION_BUILD})
    set (ROYALE_VERSION_BUILD $ENV{ROYALE_VERSION_BUILD} CACHE STRING "" FORCE)
endif()

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

set(ROYALE_VERSION "${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}.${ROYALE_VERSION_BUILD}" CACHE INTERNAL "" FORCE)

IF(ROYALE_CUSTOMER_SUFFIX)
    set(ROYALE_VERSION_NAME "${ROYALE_NAME}_${ROYALE_VERSION}_${ROYALE_CUSTOMER_SUFFIX}" CACHE INTERNAL "" FORCE)
ELSE()
    set(ROYALE_VERSION_NAME "${ROYALE_NAME}_${ROYALE_VERSION}" CACHE INTERNAL "" FORCE)
ENDIF()    

set(ROYALE_PACKAGE_NAME "${CURRENT_DATE}_${ROYALE_VERSION_NAME}" CACHE INTERNAL "" FORCE)

FILE(WRITE "${CMAKE_BINARY_DIR}/royale_version.prop" "ROYALE_PACKAGE_NAME=${ROYALE_PACKAGE_NAME}")

SET(ROYALE_VERSION_RC_FILE "${CMAKE_CURRENT_LIST_DIR}/rc/version.rc.in" CACHE INTERNAL "" FORCE)

FUNCTION(CREATE_ASSEMBLY_RC_FILE output_file)
    IF (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)  
        configure_file(
          "${ROYALE_VERSION_RC_FILE}"
          "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
          @ONLY)
          
        SET("${output_file}" "${CMAKE_CURRENT_BINARY_DIR}/version.rc" PARENT_SCOPE)
    ELSE()
        SET("${output_file}" "" PARENT_SCOPE)
    ENDIF()   
ENDFUNCTION()
