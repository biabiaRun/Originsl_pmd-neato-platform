find_package(Threads REQUIRED)

find_package(Qt5Widgets 5.5 REQUIRED)

project (rrfExportTool)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../importExportHelperLib/inc/importExportHelperLib
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/rrfExportTool.cpp"
  )

add_executable(rrfExportTool
   ${SOURCES}
   src/rrfExportTool.cpp
   )

target_link_libraries(rrfExportTool royale Qt5::Widgets)

target_link_libraries(rrfExportTool importExportHelperLib)

SET_TARGET_PROPERTIES(rrfExportTool
        PROPERTIES
        FOLDER tools)

install(TARGETS rrfExportTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)