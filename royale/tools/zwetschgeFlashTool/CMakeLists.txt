# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/config/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/storage/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/imager/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc         # import core definitions
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc/common  # import core definitions
  ${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/RegisterMapTable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseDetails.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ZwetschgeFlashTool.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/src/BridgeImagerL4.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/src/SPIStorage_M2453.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/src/SPIStorage_M2455.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/src/SPIStorageHardwareSpi.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/src/SPIStorageHelper.cpp"
  )

set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/inc/BridgeImagerL4.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/inc/SPIStorage_M2453.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/inc/SPIStorage_M2455.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/inc/SPIStorageHardwareSpi.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../spiFlashTool/inc/SPIStorageHelper.hpp"
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/RegisterMapTable.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/UseCaseDetails.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/ZwetschgeFlashTool.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/RegisterMapTable.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseDetails.ui"
  )

link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(RC_PRODUCTNAME "ZwetschgeFlashTool")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "ZwetschgeFlashTool based on Royale")
set(RC_INTERNALNAME "ZwetschgeFlashTool")
set(RC_ORIGINALFILENAME "ZwetschgeFlashTool")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

add_executable(ZwetschgeFlashTool
  ${SOURCES}
  ${HEADERS}
  ${MOC_HEADERS}
  ${UI_HEADERS}
  ${MOCED_HEADERS}
  ${ASSEMBLY_RC}
  )

SET_TARGET_PROPERTIES( ZwetschgeFlashTool
    PROPERTIES
    WIN32_EXECUTABLE ON
    )

include_directories( SYSTEM
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    )

target_link_libraries(ZwetschgeFlashTool Qt5::Widgets)

target_link_libraries(ZwetschgeFlashTool royale ${QT_LIBRARIES})

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(ZwetschgeFlashTool
    PROPERTIES
    FOLDER tools
    )

install(TARGETS ZwetschgeFlashTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
