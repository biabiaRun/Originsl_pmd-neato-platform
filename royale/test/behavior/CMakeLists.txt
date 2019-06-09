cmake_minimum_required(VERSION 2.8)

IF(ROYALE_SDK_DIR)
    project(test_royale_bdd)

    SET(CMAKE_DEBUG_POSTFIX -d)

    SET(CATCH_DIR "../../contrib/catch" CACHE STRING "" FORCE)

    OPTION(ROYALE_LOGGING      "Enable logging output globally if DEBUG is on" ON)

    message(STATUS "")
    message(STATUS "-----------------------------------------------------------")
    message(STATUS "| Project BDD testing                            (c) 2015 |")
    message(STATUS "-----------------------------------------------------------")
    message(STATUS "")

    # Get the path with no ../.. relative marks, to eliminate implicit linker warnings
    get_filename_component(SDK_LOCATION "${ROYALE_SDK_DIR}" REALPATH)
    SET(CMAKE_PREFIX_PATH "${SDK_LOCATION}/share" CACHE STRING "" FORCE)
ELSE()
    IF(ROYALE_ENABLE_BDD)
        # Not handle as own project

        include_directories(
            ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc/common
            ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc
            )
    ELSE()
        message(FATAL_ERROR "To build the BDD outside the Royale source tree, ROYALE_SDK_DIR must be set!")
    ENDIF()
ENDIF()

set(SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/Helper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TestCallbacks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TestCapturing.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TestExposure.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TestFramerate.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TestRecording.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TestUseCases.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/ContextCameraDevice.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/MockCaptureListener.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/FrameRateStats.cpp"
    )

IF ("${ROYALE_TARGET_PLATFORM}" STREQUAL "ANDROID")
    set(SOURCES ${SOURCES}
        "${CMAKE_CURRENT_LIST_DIR}/src/mainAndroid.cpp"
        )
ELSE()
    set(SOURCES ${SOURCES}
        "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
        )
ENDIF()

set(HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/inc/ContextCameraDevice.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/inc/MockCaptureListener.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/inc/FrameRateStats.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/inc/Helper.hpp"
    )

include_directories(
    ${CATCH_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/../../source/core/source/inc
    ${CMAKE_CURRENT_LIST_DIR}/../../source/core/source/inc/common
    ${CMAKE_CURRENT_LIST_DIR}/../../source/royale/inc
    ${ROYALE_SAMPLE_UTILS_FOLDER}
    )

link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

add_executable(test_royale_bdd
    ${SOURCES}
    ${HEADERS}
    )

IF(ROYALE_SDK_DIR)
    include_directories("${SDK_LOCATION}/include")
    find_package(ROYALE REQUIRED PATHS "${SDK_LOCATION}/share" NO_DEFAULT_PATH)

    FILE(GLOB inFiles RELATIVE "${SDK_LOCATION}/bin" "${SDK_LOCATION}/bin/*royale*")
    FOREACH(infileName ${inFiles})
        MESSAGE(STATUS "Process file: ${infileName}")
        ADD_CUSTOM_COMMAND(TARGET test_royale_bdd
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${SDK_LOCATION}/bin/${infileName}" $<TARGET_FILE_DIR:test_royale_bdd>
            )
    ENDFOREACH(infileName)

    IF(ROYALE_MOVE_BDD_TO)
        add_custom_command(TARGET test_royale_bdd
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:test_royale_bdd> "${ROYALE_MOVE_BDD_TO}")
    ENDIF()

    find_library(ROYALE_LIBRARY royale HINTS "${SDK_LOCATION}/bin" "${SDK_LOCATION}/lib")
    target_link_libraries(test_royale_bdd PUBLIC ${ROYALE_LIBRARY})
ELSE()
    target_link_libraries(test_royale_bdd royale)
ENDIF()

SET_TARGET_PROPERTIES(test_royale_bdd
    PROPERTIES
    FOLDER tests/bdd
    )


