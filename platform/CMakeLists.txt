#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

add_definitions(-DROYALE_ACCESS_CODE_LEVEL2="${ROYALE_ACCESS_CODE_LEVEL2}")
add_definitions(-DROYALE_ACCESS_CODE_LEVEL3="${ROYALE_ACCESS_CODE_LEVEL3}")
add_definitions(-DROYALE_ACCESS_CODE_LEVEL4="${ROYALE_ACCESS_CODE_LEVEL4}")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/core/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/core/inc/common
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/buffer/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/imager/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/config/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/processing/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/storage/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/temperature/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/factory/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/usb/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/v4l/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/royale/inc
        ${CMAKE_BINARY_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../royale/contrib/ProcessingSpectre/inc
)

if (ROYALE_ENABLE_EYESAFETY)
    add_definitions(-DROYALE_ENABLE_EYESAFETY)
endif ()

if (ROYALE_ENABLE_PLATFORM_SAMPLE)
    add_definitions(-DROYALE_FACTORY_PLAYBACK)
endif ()

link_directories(
        ${CMAKE_SYSROOT}/usr/local/lib
        ${PROCESSING_SPECTRE_LIB_DIR}
)

set(HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/BaseConfig.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/ModuleConfigCustom.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraFactory.hpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/inc/BridgeDataReceiverImpl.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/BridgeImagerImpl.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/I2cAccessImpl.hpp"
        )

set(SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/factory/src/CoreConfigFactory.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/factory/src/SoftwareDefinedImagerInterfaceAdapter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/factory/src/FlashDefinedImagerInterfaceAdapter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/factory/src/ImagerFactory.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/factory/src/ProcessingParameterMapFactory.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/config/src/CoreConfigAdapter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/config/src/FlowControlStrategyFixed.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/storage/src/StorageFormatPolar.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/storage/src/StorageI2cEeprom.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/temperature/src/TemperatureSensorTMP102.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/processing/src/ProcessingSpectre.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/processing/src/Processing.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/processing/src/ParameterMapping.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/v4l/src/BridgeV4l.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/buffer/src/OffsetBasedCapturedBuffer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/CameraRecord.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/CameraPlayback.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/FileReaderDispatcher.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/RecordedRawFrame.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/FileWriter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/FileReaderBase.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/v1/FileReader_v1.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/v2/FileReader_v2.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/v3/FileReader_v3.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../royale/source/components/record/src/UseCaseRecord.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleConfigCustom.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraFactory.cpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/src/BridgeImagerImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/I2cAccessImpl.cpp"
        )

set(SOURCES_WIN
        "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraFactory.cpp"
        )

if (NOT ROYALE_ENABLE_PLATFORM_SAMPLE)
    set (RECORD_SOURCES
            $<TARGET_OBJECTS:component_record>
        )
endif ()

if(ROYALE_ENABLE_PLATFORM_CODE)
    add_library(platform SHARED
            ${SOURCES}
            ${HEADERS}
            ${RECORD_SOURCES}
            $<TARGET_OBJECTS:component_imager>
            )

    target_link_libraries(platform
            royalecore
            spectre
            )

    SET_TARGET_PROPERTIES(platform
            PROPERTIES
            OUTPUT_NAME platform
            FOLDER modules
            VERSION ${PLATFORM_VERSION_MAJOR}.${PLATFORM_VERSION_MINOR}.${PLATFORM_VERSION_PATCH}
            )
else()
    add_library(platform SHARED
            ${SOURCES_WIN}
            ${HEADERS}
            )

    link_directories(
            ${CMAKE_SYSROOT}/usr/local/lib
            ${PROCESSING_SPECTRE_LIB_DIR}
    )

    target_link_libraries(platform royale)
    # target_link_libraries(platform debug ${ROYALE_LIB_LOCATION_DEBUG})
    # target_link_libraries(platform optimized ${ROYALE_LIB_LOCATION_RELEASE})    
endif()

FILE(GLOB PLATFORM_HEADERS inc/*)
source_group(platform FILES ${PLATFORM_HEADERS})

SET_TARGET_PROPERTIES(platform
        PROPERTIES
        OUTPUT_NAME platform
        FOLDER modules
        VERSION ${PLATFORM_VERSION_MAJOR}.${PLATFORM_VERSION_MINOR}.${PLATFORM_VERSION_PATCH}
        )

add_subdirectory(test)

INSTALL(TARGETS platform
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${ROYALE_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${ROYALE_INSTALL_LIB_DIR})

if (ROYALE_ENABLE_PLATFORM_SAMPLE)
    add_subdirectory(sampleRetrieveData)
endif ()
