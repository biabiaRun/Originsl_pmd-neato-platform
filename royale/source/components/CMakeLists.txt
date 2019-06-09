
include_directories(
    ${ROYALE_BINARY_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/imager/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/record/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/processing/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/temperature/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/factory/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/config/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/usb/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/inc
    ${PROCESSING_SPECTRE_HEADER_DIR}
    )

if (ROYALE_USE_V4L)
    include_directories (
        ${CMAKE_CURRENT_SOURCE_DIR}/v4l/inc
        )
endif()

# imager depends on storage for the generate_zwetschge_file function
add_subdirectory (storage)
add_subdirectory (imager)

add_subdirectory (buffer)
add_subdirectory (config)
add_subdirectory (factory)
add_subdirectory (processing)
add_subdirectory (record)
add_subdirectory (temperature)
add_subdirectory (usb)
if (ROYALE_USE_V4L)
    add_subdirectory (v4l)
endif()

add_subdirectory (test)
