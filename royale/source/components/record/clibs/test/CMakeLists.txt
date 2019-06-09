include_directories(
   "${ROYALE_SOURCE_DIR}/contrib/Unity/src"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc"
   )

set(CONTRIB
   "${ROYALE_SOURCE_DIR}/contrib/Unity/src/unity.c"
   )

set(HEADERS
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestFileReader.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestFileWriter.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestRecordingHelper.h"
   )

set(SOURCES
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestFileReader.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestFileReaderFileWriter.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestFileWriter.c"
   )

source_group(contrib FILES
   ${CONTRIB}
   )

add_executable(test_fileReaderFileWriter
   ${CONTRIB}
   ${HEADERS}
   ${SOURCES}
   )

target_link_libraries(test_fileReaderFileWriter rrfReaderLib rrfWriterLib)

SET_TARGET_PROPERTIES(test_fileReaderFileWriter
   PROPERTIES
   FOLDER recording/test
   )

