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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Crc32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DirtyVersion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventCaptureStream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventDeviceDisconnected.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventEyeSafety.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventForwarder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventImagerConfigNotFound.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventOverTemperature.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventProbedDevicesNotMatched.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EventRawFrameStats.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ExposureGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ExtendedData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FrameCollectorBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FrameGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NtcTempAlgo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProbedDeviceInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProbeResultInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProcessingFlag.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RawData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RawFrameSet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RoiLensCenter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleLogger.cpp"
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

set(HEADERS_COLLECTOR
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/BufferActionCalcIndividual.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/BufferActionCalcSuper.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/CapturedUseCase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/FrameCollectorBase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/IFrameCaptureListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/IFrameCaptureReleaser.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/collector/IFrameCollector.hpp"
    )

set(HEADERS_DEVICE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/CameraCore.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/CameraDevice.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/CameraDeviceBase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/ITemperatureAcceptor.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/ProbedDeviceInfo.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/ProbedDeviceInfoList.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/ProbeResultInfo.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/PsdTemperatureSensorCheckAdapter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/PsdTemperatureSensorFilter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/RoiLensCenter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/TemperatureMonitor.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/device/TemperatureSensorCheckAdapter.hpp"
    )

set(HEADERS_HAL
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/IBridgeDataReceiver.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/IBridgeImager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/IBufferCaptureListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/IBufferCaptureReleaser.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/ICapturedBuffer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/INonVolatileStorage.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/IPsdTemperatureSensor.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/hal/ITemperatureSensor.hpp"
    )

set(HEADERS_PAL
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/Access2I2cDeviceAdapter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/IDeviceControl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/IGpioAccess.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/II2cBusAccess.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/II2cDeviceAccess.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/ISpiBusAccess.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/IStorageAccessUnderlying.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/IStorageReadRandom.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/IStorageWriteFullOverwrite.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/pal/IStorageWriteSectorBased.hpp"
    )

set(HEADERS_PROCESSING
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/processing/ExtendedData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/processing/IProcessing.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/processing/IRefineExposureTime.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/processing/ProcessingParameterId.hpp"
    )

set(HEADERS_COMMON_EXCEPTIONS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/APIExceptionHandling.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/CalibrationDataNotFound.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/CouldNotOpen.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/DataNotFound.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/DeviceDetectedError.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/DeviceIsBusy.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/Disconnected.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/Exception.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/FileNotFound.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/ImagerConfigNotFoundError.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/InvalidValue.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/LogicError.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/NotImplemented.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/OutOfBounds.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/PossiblyUsbStallError.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/ResourceError.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/RuntimeError.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/Timeout.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/ValidButUnchanged.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/WrongDataFormatFound.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/exceptions/WrongState.hpp"
    )

set(HEADERS_COMMON_EVENTS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventCaptureStream.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventDeviceDisconnected.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventEyeSafety.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventImagerConfigNotFound.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventOverTemperature.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventProbedDevicesNotMatched.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/events/EventRawFrameStats.hpp"
    )

set(HEADERS_COMMON
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/Crc32.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/DirtyVersion.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/EndianConversion.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/EventForwarder.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/EventQueue.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/FileSystem.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/ICapturedRawFrame.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/IFlowControlStrategy.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/IntegerMath.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/IRImageHelper.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/IteratorBoundsCheck.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/MakeUnique.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/MsvcMacros.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/NarrowCast.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/NtcTempAlgo.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/PseudoDataTwelveBitCalculator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/RoyaleLogger.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/RoyaleProfiler.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/UuidlikeIdentifier.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/StringFunctions.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/SensorRole.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/SensorMap.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/common/WeightedAverage.hpp"
    )

set(HEADERS_ROYALE_CORE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/CallbackData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/CameraAccessLevel.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/Definitions.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/DepthData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/DepthImage.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/ExposureMode.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/FilterLevel.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/ICameraDevice.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IDepthDataListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IDepthImageListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IEvent.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IEventListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IExposureListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IExposureListener2.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IExtendedData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IExtendedDataListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IIRImageListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IPlaybackStopListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IRImage.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IRecord.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IRecordStopListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IReplay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/ISparsePointCloudListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/IntermediateData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/Iterator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/LensParameters.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/Pair.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/ProcessingFlag.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/RawData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/SparsePointCloud.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/Status.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/StreamId.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/String.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/Variant.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/royale/Vector.hpp"
    CACHE INTERNAL "")

set(HEADERS_CONFIG
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/config/ICoreConfig.hpp"
    )

set(HEADERS_USECASE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/ExposureGroup.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/FrameGroup.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/HardcodedMaxStreams.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/RawFrameSetId.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/RawFrameSet.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/Stream.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseArbitraryPhases.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseCalibration.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseDefFactoryProcessingOnly.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseDefinition.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseEightPhase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseFourPhase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseGrayScale.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseIdentifier.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseInterleavedXHt.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseMixedIrregularXHt.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseMixedXHt.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseMultiplier.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseSlave.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/usecase/UseCaseTwoPhase.hpp"
    )

# Set virtual folders for splitting includes
source_group(common                 FILES ${HEADERS_COMMON})
source_group(common\\exceptions     FILES ${HEADERS_COMMON_EXCEPTIONS})
source_group(common\\events         FILES ${HEADERS_COMMON_EVENTS})
source_group(usecase                FILES ${HEADERS_USECASE})
source_group(royale                 FILES ${HEADERS_ROYALE_CORE})
source_group(collector              FILES ${HEADERS_COLLECTOR})
source_group(config                 FILES ${HEADERS_CONFIG})
source_group(hal                    FILES ${HEADERS_HAL})
source_group(imager                 FILES ${HEADERS_IMAGER})
source_group(processing             FILES ${HEADERS_PROCESSING} ${HEADERS_PROCESSING_SPECTRE})
source_group(record                 FILES ${HEADERS_RECORD})
source_group(fileformat             FILES ${HEADER_FILE_FORMAT})

if (WIN32)
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerWindows.cpp")
elseif(APPLE)
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerApple.cpp")
elseif(UNIX)
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerLinux.cpp")
else()
    set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/RoyaleProfilerGeneric.cpp")
endif(WIN32)

add_library(${ROYALECORE_NAME} STATIC
    ${SOURCES}
    ${HEADERS_CONFIG}
    ${HEADERS_COLLECTOR}
    ${HEADERS_DEVICE}
    ${HEADERS_HAL}
    ${HEADERS_PAL}
    ${HEADERS_COMMON}
    ${HEADERS_COMMON_EXCEPTIONS}
    ${HEADERS_COMMON_EVENTS}
    ${HEADERS_USECASE}
    ${HEADERS_ROYALE_CORE}
    ${HEADERS_IMAGER}
    )

IF(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    target_link_libraries(${ROYALECORE_NAME} log)
ENDIF()
   
    
SET_TARGET_PROPERTIES(${ROYALECORE_NAME}
    PROPERTIES
    FOLDER core
    )

#IF(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
#    INSTALL(TARGETS ${ROYALECORE_NAME}
#        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
#        LIBRARY DESTINATION ${ROYALE_INSTALL_LIB_DIR}
#        ARCHIVE DESTINATION ${ROYALE_INSTALL_LIB_DIR}
#        )
#    IF(ROYALE_INSTALL_TO_MATLAB_FOLDER)
#        INSTALL(TARGETS ${ROYALECORE_NAME}
#            RUNTIME DESTINATION ${ROYALE_INSTALL_MATLAB_DIR}
#            )
#    ENDIF()
#ELSE()
#    INSTALL(TARGETS ${ROYALECORE_NAME}
#        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR}
#        LIBRARY DESTINATION ${ROYALE_INSTALL_BIN_DIR}
#        ARCHIVE DESTINATION ${ROYALE_INSTALL_BIN_DIR}
#        )
#ENDIF()
#
#IF(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
#    INSTALL(TARGETS ${ROYALECORE_NAME}
#        LIBRARY DESTINATION ${ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_DIR}
#        )
#ENDIF()


#INSTALL(FILES ${HEADERS} DESTINATION "${ROYALECORE_INSTALL_INCLUDE_DIR}/roylecore")

#IF(${ROYALECORE_TARGET_PLATFORM} STREQUAL ANDROID)
#    INSTALL(FILES ${HEADERS} DESTINATION "${ROYALECORE_INSTALL_ANDROID_EXAMPLE_APP_LIB_DIR}/roylecore")
#ENDIF()

add_subdirectory(test)

SET(ROYALECORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/" CACHE STRING "" FORCE)
