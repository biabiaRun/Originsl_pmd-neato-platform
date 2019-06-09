include_directories(
   "${CMAKE_CURRENT_SOURCE_DIR}/inc"
   "${CMAKE_CURRENT_BINARY_DIR}"
   )

DEPENDS_ON_ROYALE_VERSION_H("${CMAKE_CURRENT_SOURCE_DIR}/../src/FileWriter.cpp")

# RRF Writer library

set (RRFWRITERLIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/FileWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RRFWriter.cpp"
    )

set (RRFWRITERLIB_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../inc/record/FileWriter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/RRFWriter.h"
    ${RRF_HEADERS}
    )

if (NOT TARGET rrfWriterLib)
    add_definitions(-DRRFACCESSAPI_EXPORTS)
    add_library(rrfWriterLib SHARED
                              ${RRFWRITERLIB_SOURCES}
                              ${RRFWRITERLIB_HEADERS}
               )
    set_target_properties(rrfWriterLib
        PROPERTIES
        FOLDER recording
        )
endif ()

# File Reader library

set (RRFREADERLIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/FileReaderBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/FileReaderDispatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RRFReader.cpp"
    ${RRF_SOURCES}
    )

set (RRFREADERLIB_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../inc/record/FileReaderBase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../inc/record/FileReaderDispatcher.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/RRFReader.h"
    ${RRF_HEADERS}
    )

if (NOT TARGET rrfReaderLib)
    add_definitions(-DRRFACCESSAPI_EXPORTS)
    add_library(rrfReaderLib SHARED
                              ${RRFREADERLIB_SOURCES}
                              ${RRFREADERLIB_HEADERS}
               )
    set_target_properties(rrfReaderLib
        PROPERTIES
        FOLDER recording
        )
endif ()

add_subdirectory (test)
