IF(${ROYALE_USE_SPECTRE})
    set (PROCESSING_SPECTRE_SOURCES
        "src/ProcessingSpectre.cpp"
        "src/ParameterMapping.cpp"
        )
    set (PROCESSING_SPECTRE_HEADERS
        "inc/processing/ProcessingSpectre.hpp"
        "inc/processing/ParameterMapping.hpp"
        )
ENDIF()

set (PROCESSING_SOURCES
    "src/Processing.cpp"
    "src/ProcessingSimple.cpp"
    ${PROCESSING_SPECTRE_SOURCES}
    )
set (PROCESSING_HEADERS
    "inc/processing/Processing.hpp"
    "inc/processing/ProcessingSimple.hpp"
    ${PROCESSING_SPECTRE_HEADERS}
    )

IF(${ROYALE_USE_SPECTRE})
    set (PROCESSING_SPECTRE_TESTS
        "test/src/TestThreadedProcessingSpectre.cpp"
        )
ENDIF()

set (PROCESSING_TEST_HEADERS
    "test/inc/FixtureTestProcessing.hpp"
    "test/inc/FrameGeneratorStub.hpp"
    "test/inc/MockProcessingListeners.hpp"
    )

set (PROCESSING_TESTS
    "test/src/FixtureTestProcessing.cpp"
    "test/src/FrameGeneratorStub.cpp"
    "test/src/MockProcessingListeners.cpp"
    "test/src/TestThreadedProcessingSimple.cpp"
    ${PROCESSING_SPECTRE_TESTS}
    ${PROCESSING_TEST_HEADERS}
    )

add_library(component_processing      OBJECT ${PROCESSING_SOURCES} ${PROCESSING_HEADERS} )
set_target_properties(component_processing
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

include_directories(
    ${gtest_SOURCE_DIR}/include
    test/inc
    $<TARGET_PROPERTY:test_framework_royalecore,INCLUDE_DIRECTORIES>
    )
add_library(comptests_processing OBJECT ${PROCESSING_TESTS} )
set_target_properties(comptests_processing
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
