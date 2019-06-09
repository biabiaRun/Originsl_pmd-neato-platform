#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/system.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/version.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/packaging.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/bridgeopts.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/components.cmake)

if(ROYALE_ENABLE_COV)
  include(${CMAKE_CURRENT_LIST_DIR}/CodeCoverage.cmake)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/flags/CMakeLists.txt)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/configure)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/doc)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/sign)


configure_file(${CMAKE_CURRENT_LIST_DIR}/royale-config.cmake "${CMAKE_CURRENT_BINARY_DIR}/royale-config.cmake" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/royale-config.cmake" DESTINATION "${ROYALE_INSTALL_ROYALECONFIG_DIR}")

INSTALL(FILES "${CMAKE_CURRENT_LIST_DIR}/Modules/FindOpenNI2.cmake" DESTINATION "${ROYALE_INSTALL_ROYALECONFIG_DIR}")
