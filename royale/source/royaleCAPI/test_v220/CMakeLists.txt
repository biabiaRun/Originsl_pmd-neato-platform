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

# The first add_definitions is for v3.0.0, the second is for v2.2.0 and long term support branch.
add_definitions(-DROYALE_C_API_VERSION=220)
add_definitions(-DROYALE_NEW_API_2_2_0 -DROYALE_FINAL_API_2_2_0)
# suppress warnings about deprecated functions
if(MSVC)
   add_definitions("/wd4996")
else()
   # GCC and clang
   add_definitions("-Wno-deprecated-declarations")
endif()

add_executable(test_royaleCAPI_v220
   ${CONTRIB}
   ${HEADERS}
   ${SOURCES}
   )

target_link_libraries(test_royaleCAPI_v220 royaleCAPI)

SET_TARGET_PROPERTIES(test_royaleCAPI_v220
   PROPERTIES
   FOLDER tests/royale
   )

