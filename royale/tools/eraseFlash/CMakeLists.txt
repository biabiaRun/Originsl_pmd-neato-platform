find_package(Threads REQUIRED)

# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/imager/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/temperature/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/factory/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/config/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/usb/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/buffer/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/storage/inc
  ${ROYALE_BINARY_DIR}/source/components/imager/inc
  ${OPENGL_INCLUDE_DIR}
  ${FREEGLUT_INCLUDE_DIR}
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/EraseFlash.cpp"
  )

set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/G8PIDs.hpp"
  )

if (ROYALE_BRIDGE_ENCLUSTRA)
  list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/EraseEnclustra.cpp")
  list(APPEND HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/inc/EraseEnclustra.hpp")
endif()
if (ROYALE_BRIDGE_UVC)
  list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/EraseUvc.cpp")
  list(APPEND HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/inc/EraseUvc.hpp")
endif()

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/EraseFlash.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  )

set(COMPONENT_OBJS
    $<TARGET_OBJECTS:component_buffer>
    $<TARGET_OBJECTS:component_config>
    $<TARGET_OBJECTS:component_factory>
    $<TARGET_OBJECTS:component_imager>
    $<TARGET_OBJECTS:component_storage>
    $<TARGET_OBJECTS:component_temperature>
    $<TARGET_OBJECTS:component_usb>
    )
if(ROYALE_USE_V4L)
    set(COMPONENT_OBJS
        ${COMPONENT_OBJS}
        $<TARGET_OBJECTS:component_v4l>
        )
endif()

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(RC_PRODUCTNAME "eraseFlash")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "eraseFlash based on Royale")
set(RC_INTERNALNAME "eraseFlash")
set(RC_ORIGINALFILENAME "eraseFlash")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

add_executable(eraseFlash
  ${SOURCES}
  ${MODULE_CONFIGURATION_SOURCES}
  ${COMPONENT_OBJS}
  ${HEADERS}
  ${MOC_HEADERS}
  ${UI_HEADERS}
  ${MOCED_HEADERS}
  ${ASSEMBLY_RC}
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/factory/BridgeController.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/modules/ModuleConfigData.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/modules/UsbProbeDataListRoyale.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/src/BridgeController.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/src/UsbProbeDataListRoyale.cpp"
 
  )

SET_TARGET_PROPERTIES( eraseFlash
    PROPERTIES
    WIN32_EXECUTABLE ON
    )

include_directories( SYSTEM
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	)

target_link_libraries(eraseFlash Qt5::Widgets)

target_link_libraries(eraseFlash ${ROYALECORE_NAME} ${ROYALE_USB_LINK_COMMAND} ${QT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(eraseFlash
    PROPERTIES
    FOLDER tools
    )

IF(WIN32)
    GET_PROPERTY(link_flags TARGET eraseFlash PROPERTY LINK_FLAGS_DEBUG)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"libcmtd.lib\" /NODEFAULTLIB:\"msvcrt.lib\"")
    SET_TARGET_PROPERTIES(eraseFlash PROPERTIES LINK_FLAGS_DEBUG ${link_flags})

    GET_PROPERTY(link_flags TARGET eraseFlash PROPERTY LINK_FLAGS_RELEASE)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\"")
    SET_TARGET_PROPERTIES(eraseFlash PROPERTIES LINK_FLAGS_RELEASE ${link_flags})

    GET_PROPERTY(link_flags TARGET eraseFlash PROPERTY LINK_FLAGS_RELWITHDEBINFO)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(eraseFlash PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${link_flags})

    GET_PROPERTY(link_flags TARGET eraseFlash PROPERTY LINK_FLAGS_MINSIZEREL)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"libcmtd.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(eraseFlash PROPERTIES LINK_FLAGS_MINSIZEREL ${link_flags})

    IF(MSVC14)
        target_link_libraries(eraseFlash legacy_stdio_definitions)
    ENDIF(MSVC14)
ENDIF()

install(TARGETS eraseFlash RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
