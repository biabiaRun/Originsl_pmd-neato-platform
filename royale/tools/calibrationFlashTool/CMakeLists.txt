# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc         # import core definitions
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc/common  # import core definitions
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/storage/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/FlashDatav7.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/FlashDatav100.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/flashtool.cpp"
  )

set(HEADERS
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/FlashDataBase.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/FlashDatav7.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/FlashDatav100.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/flashtool.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/FlashDatav7.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/FlashDatav100.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  )

link_directories(
	${PROCESSING_SPECTRE_LIB_DIR}
    )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(RC_PRODUCTNAME "calibrationFlashTool")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "calibrationFlashTool based on Royale")
set(RC_INTERNALNAME "calibrationFlashTool")
set(RC_ORIGINALFILENAME "calibrationFlashTool")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

add_executable(calibrationFlashTool
  ${SOURCES}
  ${HEADERS}
  ${MOC_HEADERS}
  ${UI_HEADERS}
  ${MOCED_HEADERS}
  ${ASSEMBLY_RC}
  )

SET_TARGET_PROPERTIES( calibrationFlashTool
    PROPERTIES
    WIN32_EXECUTABLE ON
    )

include_directories( SYSTEM
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	)

target_link_libraries(calibrationFlashTool Qt5::Widgets)

target_link_libraries(calibrationFlashTool royale ${QT_LIBRARIES})

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(calibrationFlashTool
    PROPERTIES
    FOLDER tools
    )

install(TARGETS calibrationFlashTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
