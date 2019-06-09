DEPENDS_ON_ROYALE_VERSION_H("src/FileWriter.cpp")

set (RECORD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraPlayback.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CameraRecord.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FileReaderDispatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FileReaderBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/FileWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RecordedRawFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UseCaseRecord.cpp"
    )

set (V1_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/v1/FileReader_v1.cpp"
    )

set (V2_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/v2/FileReader_v2.cpp"
    )

set (V3_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/v3/FileReader_v3.cpp"
    )

set (RECORD_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/CameraPlayback.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/CameraRecord.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/CommonHeader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/FileReaderDispatcher.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/FileReaderBase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/FileWriter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/RecordedRawFrame.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/UseCaseRecord.hpp"
    )

set (V1_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/v1/FileHeader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/v1/FileReader.hpp"
    )

set (V2_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/v2/FileHeader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/v2/FileReader.hpp"
    )

set (V3_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/v3/FileHeader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/record/v3/FileReader.hpp"
    )

set (RECORD_TESTS
    "test/inc/CommonTestRecord.hpp"
    "test/src/TestCameraRecordReplay.cpp"
    "test/src/TestReaderWriter.cpp"
    )

set (RRF_HEADERS
    ${V1_HEADERS}
    ${V2_HEADERS}
    ${V3_HEADERS}
    )

set (RRF_SOURCES
    ${V1_SOURCES}
    ${V2_SOURCES}
    ${V3_SOURCES}
    )

source_group(v1 FILES     ${V1_SOURCES} ${V1_HEADERS})
source_group(v2 FILES     ${V2_SOURCES} ${V2_HEADERS})
source_group(v3 FILES     ${V3_SOURCES} ${V3_HEADERS})

# We have to add this before the include_directories call below
# otherwise we might use the wrong royale-version.h
add_subdirectory (clibs)

add_definitions(-DRRFACCESSAPI_EXPORTS)
add_library(component_record      OBJECT
    ${RECORD_SOURCES}
    ${RECORD_HEADERS}
    ${RRF_HEADERS}
    ${RRF_SOURCES}
    )
set_target_properties(component_record
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

include_directories( ${gtest_SOURCE_DIR}/include test/inc ${CMAKE_CURRENT_BINARY_DIR})
add_library(comptests_record OBJECT ${RECORD_TESTS} )
set_target_properties(comptests_record
    PROPERTIES
    FOLDER components/tests
    EXCLUDE_FROM_ALL true
    )
