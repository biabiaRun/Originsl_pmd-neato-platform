find_package(OpenGL REQUIRED)
IF(ROYALE_USE_SYSTEM_GLUT_LIBRARY)
find_package(GLUT REQUIRED)
ENDIF()

find_package(Threads REQUIRED)

if (APPLE)
   SET (FREEGLUT_INCLUDE_DIR "/opt/local/include" "usr/local/include")
   SET (GLUT_LIBRARIES glut)
   ADD_DEFINITIONS (-DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED)
   link_directories( "/opt/local/lib" "usr/local/lib" )
endif ()

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
  ${OPENGL_INCLUDE_DIR}
  ${FREEGLUT_INCLUDE_DIR}
  ${ROYALE_SAMPLE_UTILS_FOLDER}
  ${ROYALE_BINARY_DIR}/source/components/imager/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/rawviewer.cpp"
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

add_executable(rawviewer
  ${SOURCES}
  ${MODULE_CONFIGURATION_SOURCES}
  ${COMPONENT_OBJS}
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/factory/BridgeController.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/modules/ModuleConfigData.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/modules/UsbProbeDataListRoyale.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/src/BridgeController.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/src/UsbProbeDataListRoyale.cpp"
  )

IF(ROYALE_USE_SYSTEM_GLUT_LIBRARY)
    target_link_libraries(rawviewer ${ROYALECORE_NAME} ${ROYALE_USB_LINK_COMMAND} ${OPENGL_LIBRARY} ${GLUT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ELSE()
    target_link_libraries(rawviewer ${ROYALECORE_NAME} ${ROYALE_USB_LINK_COMMAND} ${OPENGL_LIBRARY} freeglut_static ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

SET_TARGET_PROPERTIES(rawviewer
    PROPERTIES
    FOLDER tools
    COMPILE_FLAGS -DFREEGLUT_STATIC
    )

IF(WIN32)
    GET_PROPERTY(link_flags TARGET rawviewer PROPERTY LINK_FLAGS_DEBUG)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"libcmtd.lib\" /NODEFAULTLIB:\"msvcrt.lib\"")
    SET_TARGET_PROPERTIES(rawviewer PROPERTIES LINK_FLAGS_DEBUG ${link_flags})

    GET_PROPERTY(link_flags TARGET rawviewer PROPERTY LINK_FLAGS_RELEASE)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\"")
    SET_TARGET_PROPERTIES(rawviewer PROPERTIES LINK_FLAGS_RELEASE ${link_flags})

    GET_PROPERTY(link_flags TARGET rawviewer PROPERTY LINK_FLAGS_RELWITHDEBINFO)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(rawviewer PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${link_flags})

    GET_PROPERTY(link_flags TARGET rawviewer PROPERTY LINK_FLAGS_MINSIZEREL)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"libcmtd.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(rawviewer PROPERTIES LINK_FLAGS_MINSIZEREL ${link_flags})
ENDIF()

install(TARGETS rawviewer RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
