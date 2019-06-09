set (V4L_TEST_HEADERS
    )

set(V4L_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestBridgeV4l.cpp"
    )

include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:test_framework_royalecore,INCLUDE_DIRECTORIES>
    )
add_library(comptests_v4l OBJECT ${V4L_TEST_SOURCES} ${V4L_TEST_HEADERS})
set_target_properties(comptests_v4l
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
