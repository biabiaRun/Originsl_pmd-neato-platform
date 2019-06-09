cmake_minimum_required (VERSION 2.8)
#project (testing_hub)

# The version number.
set (testing_hub_VERSION-MAJOR 1)
set (testing_hub_VERSION-MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/testing_hub_Config.h.in"
    "${PROJECT_BINARY_DIR}/testing_hub_Config.h"
)

include_directories("${PROJECT_BINARY_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc/royale"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/src"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc/royale"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/src"
                     ${ROYALE_SAMPLE_UTILS_FOLDER})

link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

set( SOURCES
    "src/testing_hub.cpp"
    "src/rs232.c")

# Several lines in rs232.c calculate ORs flags and then assign the result to a 16-bit variable,
# which cause Wconversion warnings.  This is C code, so the alternative to -Wno-conversions would be
# C-style casts, it can't use narrow_cast instead (even if the syntax was supported, there is no
# expectation of having to handle exceptions).
if (${ROYALE_TARGET_PLATFORM} STREQUAL LINUX)
    set_source_files_properties("src/rs232.c"
        PROPERTIES COMPILE_FLAGS "-Wno-conversion"
        )
endif()

# add the executable
add_executable(testing_hub ${SOURCES})
target_link_libraries(testing_hub royale)

SET_TARGET_PROPERTIES(testing_hub
    PROPERTIES CXX_STANDARD 11
    FOLDER tests/bdd
    )
