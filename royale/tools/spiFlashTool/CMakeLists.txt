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
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/BridgeImagerL4.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SPIFlashTool.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SPIStorage_M2452.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SPIStorage_M2453.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SPIStorage_M2455.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SPIStorageHardwareSpi.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SPIStorageHelper.cpp"
  )

set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/BridgeImagerL4.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/ImagerSPIFirmware_M2452_B1x.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIStorage_M2452.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIStorage_M2453.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIStorage_M2455.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIStorageBase.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIStorageHardwareSpi.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIStorageHelper.hpp"
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/SPIFlashTool.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  )

link_directories(
	${PROCESSING_SPECTRE_LIB_DIR}
    )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(RC_PRODUCTNAME "SPIFlashTool")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "SPIFlashTool based on Royale")
set(RC_INTERNALNAME "SPIFlashTool")
set(RC_ORIGINALFILENAME "SPIFlashTool")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

add_executable(SPIFlashTool
  ${SOURCES}
  ${HEADERS}
  ${MOC_HEADERS}
  ${UI_HEADERS}
  ${MOCED_HEADERS}
  ${ASSEMBLY_RC}
  )

SET_TARGET_PROPERTIES( SPIFlashTool
    PROPERTIES
    WIN32_EXECUTABLE ON
    )

include_directories( SYSTEM
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	)

target_link_libraries(SPIFlashTool Qt5::Widgets)

target_link_libraries(SPIFlashTool royale ${QT_LIBRARIES})

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(SPIFlashTool
    PROPERTIES
    FOLDER tools
    )

IF (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    SET_TARGET_PROPERTIES( SPIFlashTool
        PROPERTIES
        WIN32_EXECUTABLE OFF
        )
ENDIF()

install(TARGETS SPIFlashTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
