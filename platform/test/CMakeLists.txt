#****************************************************************************
# Copyright (C) 2019 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../royale/source/core/inc
    )

if(ROYALE_ENABLE_EYESAFETY)
  add_definitions(-DROYALE_ENABLE_EYESAFETY)
endif()

set(HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestPlatform.hpp"
    )

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestPlatform_General.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestPlatform_Specific.cpp"
    )

link_directories(
    ${CMAKE_SYSROOT}/usr/local/lib
    )

add_executable(test_platform
    ${HEADERS}
    ${SOURCES}
    )

target_link_libraries(test_platform platform gtest_main)

add_test(
    NAME test_platform
    COMMAND test_platform
    )

SET_TARGET_PROPERTIES(test_platform
    PROPERTIES
    FOLDER tests/platform
    )

install(TARGETS test_platform DESTINATION ${ROYALE_INSTALL_BIN_DIR})
