include_directories(
    ${gmock_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../imager/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../imager/test/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../record/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../record/test/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../factory/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../config/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../usb/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../buffer/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../temperature/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/inc/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/test/inc
    ./inc
    )

# include components unit tests
set(COMPONENT_TESTS)

if(TARGET comptests_buffer)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_buffer>)
endif()

if(TARGET comptests_config)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_config>)
endif()

if(TARGET comptests_factory)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_factory>)
endif()

if(TARGET comptests_imager)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_imager>)
endif()

if(TARGET comptests_processing)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_processing>)
endif()

if(TARGET comptests_record)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_record>)
endif()

if(TARGET comptests_storage)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_storage>)
endif()

if(TARGET comptests_temperature)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_temperature>)
endif()

if(TARGET comptests_usb)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_usb>)
endif()

if(TARGET comptests_v4l)
    list(APPEND COMPONENT_TESTS $<TARGET_OBJECTS:comptests_v4l>)
endif()

link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

add_executable(test_components
    ${COMPONENT_TESTS}
    $<TARGET_OBJECTS:test_framework_royalecore>
    )

target_link_libraries(test_components ${ROYALECORE_NAME} royale gmock_main)

add_test(
    NAME test_components
    COMMAND test_components
    )

SET_TARGET_PROPERTIES(test_components
    PROPERTIES
    FOLDER tests/components
    )

if(${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    # No test_components for the default android build;
    # the target is still available (but probably won't compile).
    SET_TARGET_PROPERTIES(test_components
        PROPERTIES
        EXCLUDE_FROM_ALL true
    )
endif()

if(ROYALE_ENABLE_COV)
    if(ROYALE_COVERAGE_GCOVR)
        SETUP_TARGET_FOR_COVERAGE_COBERTURA(
            coverage_components
            test_components
            coverage_royale
            )
    else()
        SETUP_TARGET_FOR_COVERAGE(
            coverage_components
            test_components
            coverage_components
            )
    endif()

    SET_TARGET_PROPERTIES(coverage_components
        PROPERTIES
        FOLDER coverage_components
        )
endif()
