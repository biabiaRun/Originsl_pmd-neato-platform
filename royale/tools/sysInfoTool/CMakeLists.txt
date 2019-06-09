find_package(Threads REQUIRED)

# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SysInfoTool.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/WinSystemInfo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/LinuxSystemInfo.cpp"
  )
set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SysInfoTool.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/ISystemInfo.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/WinSystemInfo.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinuxSystemInfo.hpp"
  )

link_directories(
	${PROCESSING_SPECTRE_LIB_DIR}
    )

add_executable(sysInfoTool
  ${SOURCES}
  ${HEADERS}
  )

include_directories( SYSTEM
	${Qt5Widgets_INCLUDE_DIRS}
	)

target_link_libraries(sysInfoTool Qt5::Widgets)

target_link_libraries(sysInfoTool royale ${QT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

SET_TARGET_PROPERTIES(sysInfoTool
    PROPERTIES
    FOLDER tools
    )

install(TARGETS sysInfoTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
