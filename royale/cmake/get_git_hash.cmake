#****************************************************************************
# Copyright (C) 2019 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

if (EXISTS "${REVOUTFILE}")
    file(READ "${REVOUTFILE}" oldVersionFile)

    string(REGEX MATCH "#define ROYALE_VERSION_SCM \"([a-f0-9]+)\"" oldVersionFileMatch ${oldVersionFile})
    set(OLD_GIT_REV ${CMAKE_MATCH_1})
    
    string(REGEX MATCH "#define ROYALE_VERSION_TAG \"([A-Za-z0-9_]+)\"" oldVersionFileMatch ${oldVersionFile})
    set(OLD_GIT_TAG ${CMAKE_MATCH_1})

    string(REGEX MATCH "#define ROYALE_VERSION_BRANCH \"([A-Za-z0-9_]+)\"" oldVersionFileMatch ${oldVersionFile})
    set(OLD_GIT_BRANCH ${CMAKE_MATCH_1})
else()
    set(OLD_GIT_REV "")
    set(OLD_GIT_TAG "")
    set(OLD_GIT_BRANCH "")
endif()

execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)
    
if ("${GIT_REV}" STREQUAL "")
    set(ROYALE_GIT_REV "N/A")
    set(ROYALE_GIT_TAG "N/A")
    set(ROYALE_GIT_BRANCH "N/A")
else()
    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 0 8 GIT_REV)

    if (NOT "${GIT_REV}" STREQUAL "${OLD_GIT_REV}")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --exact-match --tags
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_TAG 
            ERROR_QUIET)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            ERROR_QUIET)

        string(STRIP "${GIT_TAG}" GIT_TAG)
        string(STRIP "${GIT_BRANCH}" GIT_BRANCH)

        set(ROYALE_GIT_REV ${GIT_REV})
        set(ROYALE_GIT_TAG ${GIT_TAG})
        set(ROYALE_GIT_BRANCH ${GIT_BRANCH})
    else()
        set(ROYALE_GIT_REV ${OLD_GIT_REV})
        set(ROYALE_GIT_TAG ${OLD_GIT_TAG})
        set(ROYALE_GIT_BRANCH ${OLD_GIT_BRANCH})
    endif()
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmake/royale-version.h.in
               ${REVOUTFILE})

