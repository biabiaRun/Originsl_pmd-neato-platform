#------------------------------------------------------------------------------
# Top-level CMake file
# (c) 2016 Infineon Technologies
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.11)

set (ROYALECORE_NAME "royalecore" CACHE STRING "" FORCE)

project(${ROYALECORE_NAME})

set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

SET(CMAKE_DEBUG_POSTFIX -d)

include_directories(
    ${ROYALE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/..             
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/APIExceptionHandling.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Access2I2cDeviceAdapter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/BufferActionCalcIndividual.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/BufferActionCalcSuper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraCore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDevice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDeviceBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CapturedUseCase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CommandLog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Crc32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DirtyVersion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventCaptureStream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventDeviceDisconnected.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventEyeSafety.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventForwarder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventFrameDropped.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventImagerConfigNotFound.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventOverTemperature.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventProbedDevicesNotMatched.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventProcessing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventRawFrameStats.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventRecording.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ExposureGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ExtendedData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FileLog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FrameCollectorBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FrameGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/IlogBackend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NtcTempAlgo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProbedDeviceInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProbeResultInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProcessingFlag.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RawData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RawFrameSet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RoiLensCenter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleLogger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleTime.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SensorMap.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Status.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Stream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TemperatureMonitor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TemperatureSensorCheckAdapter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PsdTemperatureSensorCheckAdapter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PsdTemperatureSensorFilter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseArbitraryPhases.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseCalibration.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseDefFactoryProcessingOnly.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseDefinition.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseEightPhase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseFaceID.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseFourPhase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseGrayScale.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseInterleavedXHt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseMixedIrregularXHt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseMixedXHt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseMultiplier.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseSlave.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseTwoPhase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UuidlikeIdentifier.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Variant.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WeightedAverage.cpp"
    )

function(findCoreHPPs folder variable)
    file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/inc/${folder}/*.hpp")  
    set(${variable} ${hdrs}
        CACHE INTERNAL "")
endfunction(findCoreHPPs)
    
findCoreHPPs(collector ROYALECORE_HEADERS_COLLECTOR)
findCoreHPPs(device ROYALECORE_HEADERS_DEVICE)
findCoreHPPs(hal ROYALECORE_HEADERS_HAL)
findCoreHPPs(pal ROYALECORE_HEADERS_PAL)
findCoreHPPs(processing ROYALECORE_HEADERS_PROCESSING)
findCoreHPPs(common/exceptions ROYALECORE_HEADERS_COMMON_EXCEPTIONS)
findCoreHPPs(common/events ROYALECORE_HEADERS_COMMON_EVENTS)
findCoreHPPs(common ROYALECORE_HEADERS_COMMON)
findCoreHPPs(royale ROYALECORE_HEADERS_MAIN)
findCoreHPPs(config ROYALECORE_HEADERS_CONFIG)
findCoreHPPs(usecase ROYALECORE_HEADERS_USECASE)

set(ROYALECORE_HEADERS ${ROYALECORE_HEADERS_CONFIG}
                       ${ROYALECORE_HEADERS_COLLECTOR}
                       ${ROYALECORE_HEADERS_DEVICE}
                       ${ROYALECORE_HEADERS_HAL}
                       ${ROYALECORE_HEADERS_PAL}
                       ${ROYALECORE_HEADERS_COMMON}
                       ${ROYALECORE_HEADERS_COMMON_EXCEPTIONS}
                       ${ROYALECORE_HEADERS_COMMON_EVENTS}
                       ${ROYALECORE_HEADERS_USECASE}
                       ${ROYALECORE_HEADERS_MAIN}
   CACHE INTERNAL "")                    
                 
                 
if((${CMAKE_VERSION} VERSION_GREATER "3.8.0") OR 
   (${CMAKE_VERSION} VERSION_EQUAL "3.8.0"))
   # Group the headers according to their folders
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/inc PREFIX "Header Files" FILES ${ROYALECORE_HEADERS})
endif()

if (WIN32)
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerWindows.cpp")
elseif(APPLE)
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerApple.cpp")
elseif(UNIX)
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerLinux.cpp")
else()
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerGeneric.cpp")
endif(WIN32)

if(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
 list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/WindowsDebugStringLog.cpp")
 list(APPEND ROYALECORE_HEADERS_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/WindowsDebugStringLog.hpp")
endif()

if((${ROYALE_TARGET_PLATFORM} STREQUAL LINUX) OR (ROYALE_TARGET_PLATFORM STREQUAL "APPLE"))
 list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/SysLogger.cpp")
 list(APPEND ROYALECORE_HEADERS_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/SysLogger.hpp")
endif()

if(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
 list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/AndroidLog.cpp")
 list(APPEND ROYALECORE_HEADERS_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/AndroidLog.hpp")
endif()

add_library(${ROYALECORE_NAME} STATIC
    ${SOURCES}
    ${ROYALECORE_HEADERS}
    )

IF(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    target_link_libraries(${ROYALECORE_NAME} log)
ENDIF()
   
    
SET_TARGET_PROPERTIES(${ROYALECORE_NAME}
    PROPERTIES
    FOLDER core
    )

add_subdirectory(test)

SET(ROYALECORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/" CACHE STRING "" FORCE)
