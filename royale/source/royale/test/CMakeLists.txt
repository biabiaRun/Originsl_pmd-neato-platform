include_directories(
   ${gmock_SOURCE_DIR}/include
   ${gtest_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/inc
   ${CMAKE_CURRENT_SOURCE_DIR}/../../core/test/inc
   ${ROYALE_SAMPLE_UTILS_FOLDER}
   )

# There are two test projects here - test_royale requires a hardware camera to be attached,
# and isn't run automatically by CMake's "test" target.

set(UNIT_TEST_SOURCES
   "${CMAKE_CURRENT_SOURCE_DIR}/src/UnitTestCameraManager.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/UnitTestModuleConfigData.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/UnitTestRoyale.cpp"
   )

set(HARDWARE_TEST_SOURCES
   "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDeviceL1Fixture.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDeviceL2Fixture.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDeviceL3Fixture.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraDeviceL4Fixture.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDevice.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL2.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL3.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL4.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraManager.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestListeners.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRecordReplay.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRecordReplayL2.cpp"
   )

set(HEADERS
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraDeviceL1Fixture.hpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraDeviceL2Fixture.hpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraDeviceL3Fixture.hpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/CameraDeviceL4Fixture.hpp"
   )

# The unit tests can use this to load test data, independent of the current working directory that
# the test is run from.  String literals are concated when side-by-side in the source, so
#    const royale::String rrfFilename{ ROYALE_TEST_FILE_PATH "/ListenerTest.rrf" };
# is handled by the compiler.
ADD_DEFINITIONS(-DROYALE_TEST_FILE_PATH="${ROYALE_RUNTIME_OUTPUT_DIRECTORY}")

add_executable(test_royale_without_hardware
   ${UNIT_TEST_SOURCES}
   ${HEADERS}
   $<TARGET_OBJECTS:test_framework_royalecore>
   )

target_link_libraries(test_royale_without_hardware royale gmock_main)

add_test(
    NAME test_royale_without_hardware
    COMMAND test_royale_without_hardware
    )

SET_TARGET_PROPERTIES(test_royale_without_hardware
   PROPERTIES
   FOLDER tests/royale
   )

add_executable(test_royale
   ${UNIT_TEST_SOURCES}
   ${HARDWARE_TEST_SOURCES}
   ${HEADERS}
   $<TARGET_OBJECTS:test_framework_royalecore>
   )

target_link_libraries(test_royale royale gmock_main)

SET_TARGET_PROPERTIES(test_royale
   PROPERTIES
   FOLDER tests/royale
   )

add_custom_command(
    TARGET test_royale POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/ListenerTest.rrf
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/ListenerTest.rrf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/ListenerTest_Depth.dat
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/ListenerTest_Depth.dat
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/ListenerTest_IR.dat
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/ListenerTest_IR.dat
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/ListenerTest_SPC.dat
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/ListenerTest_SPC.dat
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/rrf_v1.rrf
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/rrf_v1.rrf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/rrf_v2.rrf
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/rrf_v2.rrf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/files/rrf_v3.rrf
            ${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/rrf_v3.rrf
    )   

# Add the POST_BUILD command above as a dependency of test_royale_without_hardware
add_dependencies(test_royale_without_hardware
    test_royale
    )
  
if(ROYALE_ENABLE_COV)
   if(ROYALE_COVERAGE_GCOVR)
      SETUP_TARGET_FOR_COVERAGE_COBERTURA(
         coverage_royale
         test_royale
         coverage_royale
         )
   else()
      SETUP_TARGET_FOR_COVERAGE(
         coverage_royale
         test_royale
         coverage_royale
         )
   endif()

   SET_TARGET_PROPERTIES(coverage_royale
      PROPERTIES
      FOLDER coverage_royale
      )
endif()
