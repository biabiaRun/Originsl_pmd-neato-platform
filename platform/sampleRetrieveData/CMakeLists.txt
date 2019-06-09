#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../royale/source/core/inc
    )

set(HEADERS
    )
    
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sampleRetrieveData.cpp"
    )

link_directories(
    ${CMAKE_SYSROOT}/usr/local/lib
    )

add_executable(sampleRetrieveData
    ${HEADERS}
    ${SOURCES}
    )

install(TARGETS sampleRetrieveData DESTINATION ${ROYALE_INSTALL_BIN_DIR})

target_link_libraries(sampleRetrieveData platform)
