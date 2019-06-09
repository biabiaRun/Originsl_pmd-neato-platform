include_directories(
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )

set (CORE_TEST_FRAMEWORK_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/ThreadedAssertSupport.hpp"
    )

set (HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/BufferGeneratorPregen.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/BufferGeneratorStub.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/FixtureTestFrameCollector.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/MockFrameCaptureListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/StubTemperatureSensor.hpp"
    )

set(CORE_TEST_FRAMEWORK_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadedAssertSupport.cpp"
    )

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/BufferGeneratorStub.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FixtureTestFrameCollector.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/MockFrameCaptureListener.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraCore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCrc32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestDepthData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestEndianConversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestEventForwarder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestEventQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestEvents.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestExceptions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestFileSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestFrameCollectorIndividual.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestFrameCollectorSuper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestIntegerMath.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestIteratorBoundsCheck.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestLogger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestNtcTempAlgo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestProcessingFlag.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestPsdTemperatureSensorCheckAdapter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestPsdTemperatureSensorFilter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRoiLensCenter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRoyaleProfiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestSensorRoutingConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestStatus.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestString.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestTemperatureMonitor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestTemperatureSensorCheckAdapter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUseCase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUseCaseArbitraryPhases.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUseCaseDefFactoryProcessingOnly.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUseCaseDefinition.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUseCaseFourPhase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUseCaseSlave.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestUuidlikeIdentifier.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestVariant.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestVectorPair.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestWeightedAverage.cpp"
    )

add_library(test_framework_royalecore OBJECT
    ${CORE_TEST_FRAMEWORK_HEADERS}
    ${CORE_TEST_FRAMEWORK_SOURCES}
    )
set_target_properties(test_framework_royalecore
    PROPERTIES
    FOLDER core
    EXCLUDE_FROM_ALL true
    )
target_include_directories(test_framework_royalecore
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )

add_executable(test_royalecore
    $<TARGET_OBJECTS:test_framework_royalecore>
    ${HEADERS}
    ${SOURCES}
    )

SET_TARGET_PROPERTIES(test_royalecore
    PROPERTIES
    FOLDER core
    )

target_link_libraries(test_royalecore ${ROYALECORE_NAME} gmock_main)

add_test(
    NAME test_royalecore
    COMMAND test_royalecore
    )

if(ROYALE_ENABLE_COV)
    if(ROYALE_COVERAGE_GCOVR)
        SETUP_TARGET_FOR_COVERAGE_COBERTURA(
            coverage_test_royalecore
            test_royalecore
            coverage_test_royalecore
            )
    else()
        SETUP_TARGET_FOR_COVERAGE(
            coverage_test_royalecore
            test_royalecore
            coverage_test_royalecore
            )
    endif()

    SET_TARGET_PROPERTIES(coverage_test_royalecore
        PROPERTIES
        FOLDER coverage_test_royalecore
        )
endif()
