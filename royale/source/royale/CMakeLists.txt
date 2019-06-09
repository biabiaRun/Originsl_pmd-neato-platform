include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/private
    ${CMAKE_CURRENT_SOURCE_DIR}/../platform/inc
    ${ROYALE_BINARY_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/imager/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/record/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/processing/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/temperature/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/factory/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/config/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/usb/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/buffer/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/storage/inc
    ${PROCESSING_SPECTRE_HEADER_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROYALE_BINARY_DIR}/source/components/imager/inc
    )
if(ROYALE_USE_V4L)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../components/v4l/inc
        )
endif()

link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )


if (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    include_directories(
        ${LIBUSBFOLDER}
        )
endif (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)

set(HEADERS_PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/DeviceFactory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/imager/ImagerBaseConfigEvalboard.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/imager/ImagerBaseConfigPicoFlexx.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/imager/ImagerBaseConfigPicoMonstar.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/CommonProcessingParameters.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactoryAnimatorBoard.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactoryOrpheus.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactoryPicoFlexx.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactoryPicoMonstar.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactoryPmdModule238x.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactoryPmdModule277x.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactorySalome.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigFactorySelene.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigOrpheusCommon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigPicoFlexxCommon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigPicoMonstarCommon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigPmdModuleCommon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigSalomeCommon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigSeleneCommon.hpp"
    )

set(HEADERS_TOP
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale.hpp"
    )

set(HEADERS_ROYALE
    ${HEADERS_ROYALE_CORE}
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/factory/BridgeController.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/CameraManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/CharacterizationHelper.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/ModuleConfigData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/TriggerMode.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/modules/UsbProbeDataListRoyale.hpp"
    )

DEPENDS_ON_ROYALE_VERSION_H("${CMAKE_CURRENT_SOURCE_DIR}/src/royale.cpp")

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/BridgeController.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DeviceFactory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UsbProbeDataListRoyale.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/royale.cpp"
    )

set(MODULE_CONFIGURATION_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CharacterizationHelper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataAnimatorDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataMTT016.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataOrat45.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataOrpheus.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataOrpheusDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoFlexxU6.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoMonstar1.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoMonstar2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoMonstar850nmGlass.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoMonstar850nmGlass2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoMonstar940nm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPicoMonstarDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPmdModule238xDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataPmdModule277xDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataSalome940nm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataSalome2Rev940nm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataSalomeDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataSeleneDefault.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataSeleneIcm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigDataX18502W.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactoryAnimatorBoard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactoryOrpheus.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactoryPicoFlexx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactoryPicoMonstar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactoryPmdModule238x.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactoryPmdModule277x.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactorySalome.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigFactorySelene.cpp"
    CACHE INTERNAL "" FORCE
    )

set(RC_PRODUCTNAME "libroyale")
set(RC_FILETYPE "VFT_DLL")
set(RC_FILEDESCRIPTION "Software framework for time-of-flight cameras")
set(RC_INTERNALNAME "libroyale")
set(RC_ORIGINALFILENAME "Royale")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

set(SPECTRE_LIB "spectre")

IF(NOT ${ROYALE_USE_SPECTRE})
    set(HEADERS_PROCESSING_SPECTRE "")
    set(SOURCES_PROCESSING_SPECTRE "")
    set(SPECTRE_LIB "")
ENDIF()

set(COMPONENT_OBJS
    $<TARGET_OBJECTS:component_buffer>
    $<TARGET_OBJECTS:component_config>
    $<TARGET_OBJECTS:component_factory>
    $<TARGET_OBJECTS:component_imager>
    $<TARGET_OBJECTS:component_processing>
    $<TARGET_OBJECTS:component_record>
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

add_library(royale SHARED
    ${MODULE_CONFIGURATION_SOURCES}
    ${SOURCES}
    ${COMPONENT_OBJS}
    ${HEADERS_PRIVATE}
    ${HEADERS_TOP}
    ${HEADERS_ROYALE}
    ${ASSEMBLY_RC}
    )

FILE (GLOB ROYALE_HEADERS          inc/royale/*)
FILE (GLOB ROYALE_HEADERS_TOP      inc/*)
set(ROYALE_HEADERS ${ROYALE_HEADERS} ${HEADERS_ROYALE})
source_group(royale FILES          ${ROYALE_HEADERS})
source_group(royale FILES          ${ROYALE_HEADERS_TOP})
source_group(private FILES          ${HEADERS_PRIVATE})
source_group(processing FILES      ${HEADERS_PROCESSING_SPECTRE})

target_link_libraries(royale
    ${ROYALE_USB_LINK_COMMAND}
    ${ROYALECORE_NAME}
    ${SPECTRE_LIB}
  )

IF(MSVC14)
    target_link_libraries(royale legacy_stdio_definitions)
ENDIF()

IF(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    target_link_libraries(royale log)
ENDIF()

SET_TARGET_PROPERTIES(royale
    PROPERTIES
    OUTPUT_NAME royale
    FOLDER modules
    VERSION ${ROYALE_VERSION_MAJOR}.${ROYALE_VERSION_MINOR}.${ROYALE_VERSION_PATCH}
    )

IF(WIN32)
    GET_PROPERTY(link_flags TARGET royale PROPERTY LINK_FLAGS_DEBUG)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"libcmtd.lib\" /NODEFAULTLIB:\"msvcrt.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(royale PROPERTIES LINK_FLAGS_DEBUG ${link_flags})

    GET_PROPERTY(link_flags TARGET royale PROPERTY LINK_FLAGS_RELEASE)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(royale PROPERTIES LINK_FLAGS_RELEASE ${link_flags})

    GET_PROPERTY(link_flags TARGET royale PROPERTY LINK_FLAGS_RELWITHDEBINFO)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(royale PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${link_flags})

    GET_PROPERTY(link_flags TARGET royale PROPERTY LINK_FLAGS_MINSIZEREL)
    SET(link_flags "${link_flags} /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"libcmtd.lib\" /ignore:4099")
    SET_TARGET_PROPERTIES(royale PROPERTIES LINK_FLAGS_MINSIZEREL ${link_flags})
ENDIF()

IF(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    INSTALL(TARGETS royale
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${ROYALE_INSTALL_LIB_DIR}
        )
        
    INSTALL(TARGETS royale
        RUNTIME DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        LIBRARY DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        ARCHIVE DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData"
        )

    INSTALL(TARGETS royale
        RUNTIME DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        LIBRARY DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        ARCHIVE DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo"
        )
        
    IF(${ROYALE_USE_SPECTRE})
        get_target_property(SPECTRE_LIB spectre IMPORTED_LOCATION)
        INSTALL(FILES ${SPECTRE_LIB} DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetRetrieveData")
        INSTALL(FILES ${SPECTRE_LIB} DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/csharp/sampleDotNetCamInfo")
    ENDIF()
        
ELSE()
    INSTALL(TARGETS royale
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        ARCHIVE DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        )
ENDIF()
        
IF(UNIX AND
   NOT ${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    INSTALL(TARGETS royale
        RUNTIME DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/cpp/sampleROS/royale/bin"
        LIBRARY DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/cpp/sampleROS/royale/bin"
        ARCHIVE DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/cpp/sampleROS/royale/bin"
        )

    IF(${ROYALE_USE_SPECTRE})
        get_target_property(SPECTRE_LIB spectre IMPORTED_LOCATION)
        INSTALL(FILES ${SPECTRE_LIB} DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/cpp/sampleROS/royale/bin")
    ENDIF()
ENDIF()

IF(ROYALE_INSTALL_TO_MATLAB_FOLDER)
    INSTALL(TARGETS royale
        RUNTIME DESTINATION ${ROYALE_INSTALL_MATLAB_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_MATLAB_DIR}
        )
    IF(${ROYALE_USE_SPECTRE})
        get_target_property(SPECTRE_LIB spectre IMPORTED_LOCATION)
        INSTALL(FILES ${SPECTRE_LIB} DESTINATION "${ROYALE_INSTALL_MATLAB_DIR}")
    ENDIF()
ENDIF()

IF(ROYALE_INSTALL_TO_PYTHON_FOLDER)
    INSTALL(TARGETS royale
        RUNTIME DESTINATION ${ROYALE_INSTALL_PYTHON_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_PYTHON_DIR}
        )
    IF(${ROYALE_USE_SPECTRE})
        get_target_property(SPECTRE_LIB spectre IMPORTED_LOCATION)
        INSTALL(FILES ${SPECTRE_LIB} DESTINATION "${ROYALE_INSTALL_PYTHON_DIR}")
    ENDIF()
ENDIF()

IF(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    INSTALL(TARGETS royale
        LIBRARY DESTINATION ${ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_DIR}
        )

    IF(${ROYALE_USE_SPECTRE})
        get_target_property(SPECTRE_LIB spectre IMPORTED_LOCATION)
        INSTALL(FILES ${SPECTRE_LIB} DESTINATION ${ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_DIR})
    ENDIF()
        
    INSTALL(FILES ${HEADERS_ROYALE} DESTINATION "${ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_HEADER_DIR}/royale")
    INSTALL(FILES ${HEADERS_TOP}    DESTINATION "${ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_HEADER_DIR}")
ENDIF()

IF(${ROYALE_USE_SPECTRE})
    get_target_property(SPECTRE_LIB spectre IMPORTED_LOCATION)
    INSTALL(FILES ${SPECTRE_LIB} DESTINATION ${ROYALE_INSTALL_BIN_DIR})
ENDIF()

INSTALL(FILES ${HEADERS_ROYALE} DESTINATION "${ROYALE_INSTALL_INCLUDE_DIR}/royale")
INSTALL(FILES ${HEADERS_TOP}    DESTINATION "${ROYALE_INSTALL_INCLUDE_DIR}")

IF(UNIX AND
   NOT ${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    INSTALL(FILES ${HEADERS_ROYALE} DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/cpp/sampleROS/royale/include/royale")
    INSTALL(FILES ${HEADERS_TOP}    DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/cpp/sampleROS/royale/include")
ENDIF()

if(NOT ${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_subdirectory(test)
endif()
