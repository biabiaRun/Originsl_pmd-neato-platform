
set (BUFFER_SOURCES
    "src/BridgeCopyAndNormalize.cpp"
    "src/BridgeInternalBufferAlloc.cpp"
    "src/OffsetBasedCapturedBuffer.cpp"
    "src/SimpleCapturedBuffer.cpp"
    )

set (BUFFER_HEADERS
    "inc/buffer/BridgeCopyAndNormalize.hpp"
    "inc/buffer/BridgeInternalBufferAlloc.hpp"
    "inc/buffer/BufferDataFormat.hpp"
    "inc/buffer/BufferUtils.hpp"
    "inc/buffer/OffsetBasedCapturedBuffer.hpp"
    "inc/buffer/SimpleCapturedBuffer.hpp"
    )

set (BUFFER_TESTS
    "test/TestBridgeInternalBufferAlloc.cpp"
    "test/TestBufferUtils.cpp"
    )

add_library(component_buffer      OBJECT ${BUFFER_SOURCES} ${BUFFER_HEADERS} )
set_target_properties(component_buffer
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

include_directories( ${gtest_SOURCE_DIR}/include test/inc )
add_library(comptests_buffer OBJECT ${BUFFER_TESTS} )
set_target_properties(comptests_buffer
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
