# Using Qt for command line parsing
find_package(Qt5Widgets REQUIRED)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc
  ${Qt5Widgets_INCLUDE_DIRS}
  )

if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
  set (SOURCES_OSDEP
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RunProgramWin.cpp"
    )
else()
  set (SOURCES_OSDEP
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RunProgramUnix.cpp"
    )
endif()
  
set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/each_camera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraBlocker.cpp"
  "${SOURCES_OSDEP}"
  )

set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraBlocker.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/RunProgram.hpp"
  )

add_executable(each_camera
  ${SOURCES}
  ${HEADERS}
  )

IF(WIN32)
    set(EXTRA_LINK_CMD "shlwapi")
endif()

target_link_libraries(each_camera royale ${EXTRA_LINK_CMD})

target_link_libraries(each_camera Qt5::Widgets)

SOURCE_GROUP( "inc" FILES ${HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(each_camera
    PROPERTIES
    FOLDER tools
    )

install(TARGETS each_camera RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
