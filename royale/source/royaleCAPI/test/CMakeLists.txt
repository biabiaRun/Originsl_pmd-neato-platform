include_directories(
   "${ROYALE_SOURCE_DIR}/contrib/Unity/src"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc"
   ${ROYALE_SAMPLE_UTILS_FOLDER}
   )

set(CONTRIB
   "${ROYALE_SOURCE_DIR}/contrib/Unity/src/unity.c"
   )

set(HEADERS
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestHelpers.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestCameraDeviceL1.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestCameraDeviceL2.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestCameraDeviceL3.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/inc/TestCameraDeviceL4.h"
   )

set(SOURCES
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestHelpers.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL1.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL2.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL3.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/TestCameraDeviceL4.c"
   "${CMAKE_CURRENT_SOURCE_DIR}/src/test_royaleCAPI.c"
   )

source_group(contrib FILES
   ${CONTRIB}
   )

add_executable(test_royaleCAPI
   ${CONTRIB}
   ${HEADERS}
   ${SOURCES}
   )

target_link_libraries(test_royaleCAPI royaleCAPI)

SET_TARGET_PROPERTIES(test_royaleCAPI
   PROPERTIES
   FOLDER tests/royale
   )

