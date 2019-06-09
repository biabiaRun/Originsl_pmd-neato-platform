set (CONFIG_TEST_HEADERS
    )

set(CONFIG_TEST_SOURCES
    "src/TestExternalConfig.cpp"
    )

include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:test_framework_royalecore,INCLUDE_DIRECTORIES>
    )

add_library(comptests_config OBJECT ${CONFIG_TEST_SOURCES} ${CONFIG_TEST_HEADERS})
set_target_properties(comptests_config
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
