# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/record/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/record/clibs/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/rrftool.cpp"
  )

set(HEADERS
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/rrftool.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  )

link_directories(
	${PROCESSING_SPECTRE_LIB_DIR}
    )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(RC_PRODUCTNAME "rrfTool")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "rrfTool based on Royale")
set(RC_INTERNALNAME "rrfTool")
set(RC_ORIGINALFILENAME "rrfTool")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

add_executable(rrfTool
  ${SOURCES}
  ${HEADERS}
  ${MOC_HEADERS}
  ${UI_HEADERS}
  ${MOCED_HEADERS}
  ${ASSEMBLY_RC}
  )

SET_TARGET_PROPERTIES( rrfTool
    PROPERTIES
    WIN32_EXECUTABLE ON
    )

include_directories( SYSTEM
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	)

target_link_libraries(rrfTool Qt5::Widgets)

target_link_libraries(rrfTool rrfReaderLib rrfWriterLib ${QT_LIBRARIES})

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(rrfTool
    PROPERTIES
    FOLDER tools
    )

install(TARGETS rrfTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
