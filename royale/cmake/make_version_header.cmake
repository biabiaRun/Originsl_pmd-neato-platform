#****************************************************************************
# Copyright (C) 2019 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

ROYALE_FIND_HOST_PACKAGE(Git)

execute_process(COMMAND "${CMAKE_COMMAND}"
                -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                -D ROYALE_VERSION_MAJOR=${ROYALE_VERSION_MAJOR}
                -D ROYALE_VERSION_MINOR=${ROYALE_VERSION_MINOR}
                -D ROYALE_VERSION_PATCH=${ROYALE_VERSION_PATCH}
                -D ROYALE_VERSION_BUILD=${ROYALE_VERSION_BUILD}
                -D ROYALE_VERSION_CUSTOMER_SUFFIX=${ROYALE_CUSTOMER_SUFFIX}
                -D REVOUTFILE=${CMAKE_BINARY_DIR}/royale-version.h
                -P ${ROYALE_SOURCE_DIR}/cmake/get_git_hash.cmake
                WORKING_DIRECTORY ${ROYALE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_REV)
