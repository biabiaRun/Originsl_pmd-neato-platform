# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_SOURCE_DIR}/source/components
  ${CMAKE_SOURCE_DIR}/source/components/config/inc
  ${CMAKE_SOURCE_DIR}/source/components/factory/inc
  ${CMAKE_SOURCE_DIR}/source/components/imager/inc
  ${CMAKE_SOURCE_DIR}/source/components/record/inc
  ${CMAKE_SOURCE_DIR}/source/royale/inc
  ${CMAKE_SOURCE_DIR}/source/core/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/raw2rrf.cpp"
  )

set(HEADERS
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/raw2rrf.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  )

link_directories(
    )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(RC_PRODUCTNAME "raw2rrf")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "raw2rrf based on Royale")
set(RC_INTERNALNAME "raw2rrf")
set(RC_ORIGINALFILENAME "raw2rrf")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

add_executable(raw2rrf
  ${SOURCES}
  ${HEADERS}
  ${MOC_HEADERS}
  ${UI_HEADERS}
  ${MOCED_HEADERS}
  ${ASSEMBLY_RC}
  )

SET_TARGET_PROPERTIES( raw2rrf
    PROPERTIES
    WIN32_EXECUTABLE ON
    )

include_directories( SYSTEM
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    )

target_link_libraries(raw2rrf Qt5::Widgets)

target_link_libraries(raw2rrf royale)

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(raw2rrf
    PROPERTIES
    FOLDER tools
    )

install(TARGETS raw2rrf RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
