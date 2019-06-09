#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

set(SAMPLE_CPP_SUBFOLDER cpp)
set(SAMPLE_C_SUBFOLDER c)
set(SAMPLE_CSHARP_SUBFOLDER csharp)
set(SAMPLE_ANDROID_SUBFOLDER android)

set(ROYALE_SAMPLE_UTILS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/inc/sample_utils" CACHE STRING "Folder of the sample utilities" FORCE)

# Stuff to install the samples in the package
IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    configure_file(android/sampleNativeAndroid/app/build.gradle.template
            "${CMAKE_CURRENT_BINARY_DIR}/android/sampleNativeAndroid/app/build.gradle")
    INSTALL(DIRECTORY android DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}" PATTERN "*.template" EXCLUDE)
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/android/sampleNativeAndroid/app/build.gradle"
            DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_ANDROID_SUBFOLDER}/sampleNativeAndroid/app")
ELSE ()
    INSTALL(FILES CMakeLists.template DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}" RENAME CMakeLists.txt)
    INSTALL(FILES README_samples.md DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}")
    INSTALL(DIRECTORY inc DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}")

    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleCameraInfo DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleExportPLY DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleIReplay DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleMultiCamera DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleOpenCV DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleRecordRRF DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleRetrieveData DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleSlave DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")

    INSTALL(DIRECTORY ${SAMPLE_C_SUBFOLDER}/sampleCAPI DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_C_SUBFOLDER}")

    IF (MSVC)
        INSTALL(DIRECTORY ${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetRetrieveData DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CSHARP_SUBFOLDER}")
        INSTALL(DIRECTORY ${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetCamInfo DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CSHARP_SUBFOLDER}")
    ENDIF ()

    IF (UNIX)
        INSTALL(DIRECTORY ${SAMPLE_CPP_SUBFOLDER}/sampleROS DESTINATION "${ROYALE_INSTALL_SAMPLES_DIR}/${SAMPLE_CPP_SUBFOLDER}")
    ENDIF ()
ENDIF ()

add_subdirectory(python)

# If we don't want the samples to be built return at this point
IF ((NOT ROYALE_ENABLE_SAMPLES) OR (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID))
    RETURN()
ENDIF ()

link_directories(${PROCESSING_SPECTRE_LIB_DIR})

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/core/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/royale/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/royaleCAPI/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Sample Camera Info
#------------------------------------------------

add_executable(sampleCameraInfo
        ${SAMPLE_CPP_SUBFOLDER}/sampleCameraInfo/sampleCameraInfo.cpp)

target_link_libraries(sampleCameraInfo royale)

SET_TARGET_PROPERTIES(sampleCameraInfo
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleCameraInfo
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample Record RRF
#------------------------------------------------

add_executable(sampleRecordRRF ${SAMPLE_CPP_SUBFOLDER}/sampleRecordRRF/sampleRecordRRF.cpp)

target_link_libraries(sampleRecordRRF royale)

SET_TARGET_PROPERTIES(sampleRecordRRF
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleRecordRRF
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample Retrieve Data
#------------------------------------------------

add_executable(sampleRetrieveData
        ${SAMPLE_CPP_SUBFOLDER}/sampleRetrieveData/sampleRetrieveData.cpp)

target_link_libraries(sampleRetrieveData royale)

SET_TARGET_PROPERTIES(sampleRetrieveData
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleRetrieveData
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample Export PLY
#------------------------------------------------

add_executable(sampleExportPLY ${SAMPLE_CPP_SUBFOLDER}/sampleExportPLY/sampleExportPLY.cpp)

target_link_libraries(sampleExportPLY royale)

SET_TARGET_PROPERTIES(sampleExportPLY
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleExportPLY
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample IReplay
#------------------------------------------------

add_executable(sampleIReplay ${SAMPLE_CPP_SUBFOLDER}/sampleIReplay/sampleIReplay.cpp)

target_link_libraries(sampleIReplay royale)

set_target_properties(sampleIReplay
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleIReplay RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample Multi Camera
#------------------------------------------------

add_executable(sampleMultiCamera ${SAMPLE_CPP_SUBFOLDER}/sampleMultiCamera/sampleMultiCamera.cpp)

target_link_libraries(sampleMultiCamera royale)

SET_TARGET_PROPERTIES(sampleMultiCamera
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleMultiCamera RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample Slave
#------------------------------------------------

add_executable(sampleSlave ${SAMPLE_CPP_SUBFOLDER}/sampleSlave/sampleSlave.cpp)

target_link_libraries(sampleSlave royale)

SET_TARGET_PROPERTIES(sampleSlave
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleSlave RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample Processing Parameters
#------------------------------------------------

add_executable(sampleProcessingParameters ${SAMPLE_CPP_SUBFOLDER}/sampleProcessingParameters/sampleProcessingParameters.cpp)

target_link_libraries(sampleProcessingParameters royale)

SET_TARGET_PROPERTIES(sampleProcessingParameters
        PROPERTIES
        FOLDER samples)

# Sample Raw Data
#------------------------------------------------

add_executable(sampleRawData ${SAMPLE_CPP_SUBFOLDER}/sampleRawData/sampleRawData.cpp)

target_link_libraries(sampleRawData royale)

SET_TARGET_PROPERTIES(sampleRawData
        PROPERTIES
        FOLDER samples)

# Sample C API
#------------------------------------------------

add_executable(sampleCAPI ${SAMPLE_C_SUBFOLDER}/sampleCAPI/sampleCAPI.c)

target_link_libraries(sampleCAPI royaleCAPI)

SET_TARGET_PROPERTIES(sampleCAPI
        PROPERTIES
        FOLDER samples)

install(TARGETS sampleCAPI RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})

# Sample DotNet
#------------------------------------------------

if (MSVC)
    project(sampleDotNetRetrieveData)

    string(REGEX REPLACE "/" "\\\\" ROYALE_DOT_NET_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_CSHARP_SUBFOLDER}")
    set(ROYALE_DOT_NET_SRC_DIR "${ROYALE_DOT_NET_SRC_DIR}\\sampleDotNetRetrieveData")

    configure_file(
            "${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetRetrieveData/sampleDotNetRetrieveDataInternal.csproj.in"
            "${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetRetrieveData.csproj")

    include_external_msproject(sampleDotNetRetrieveData
            ${CMAKE_CURRENT_BINARY_DIR}/${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetRetrieveData.csproj
            PLATFORM ${MACHINE}
            RoyaleDotNet)

    set_target_properties(sampleDotNetRetrieveData
            PROPERTIES
            OUTPUT_NAME sampleDotNetRetrieveData
            FOLDER samples)

    install(PROGRAMS ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/sampleDotNetRetrieveData.exe
            DESTINATION "${ROYALE_INSTALL_BIN_DIR}")


    # Sample Dot Net CameraInfo
    # -------------------------
    project(sampleDotNetCamInfo)


    string(REGEX REPLACE "/" "\\\\" ROYALE_DOT_NET_SRC_DIR_CAMINFO "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_CSHARP_SUBFOLDER}")
    set(ROYALE_DOT_NET_SRC_DIR_CAMINFO "${ROYALE_DOT_NET_SRC_DIR_CAMINFO}\\sampleDotNetCamInfo")

    configure_file(
            "${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetCamInfo/sampleDotNetCamInfoInternal.csproj.in"
            "${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetCamInfo.csproj")

    include_external_msproject(sampleDotNetCamInfo
            ${CMAKE_CURRENT_BINARY_DIR}/${SAMPLE_CSHARP_SUBFOLDER}/sampleDotNetCamInfo.csproj
            PLATFORM ${MACHINE}
            RoyaleDotNet)

    set_target_properties(sampleDotNetCamInfo
            PROPERTIES
            OUTPUT_NAME sampleDotNetCamInfo
            FOLDER samples)

    install(PROGRAMS ${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/sampleDotNetCamInfo.exe
            DESTINATION "${ROYALE_INSTALL_BIN_DIR}")

endif ()

# Sample OpenCV
#------------------------------------------------

find_package(OpenCV QUIET)

if (NOT OpenCV_FOUND)
    message("OpenCV example will not be built as no OpenCV was found!")
else ()
    set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
    set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE)
    if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
        # todo ROYAL-2871 Check compilation on Windows, which will need the OpenCV_STATIC and
        # CMAKE_TRY_COMPILE_CONFIGURATION to be considered. For now, assume the test compilation
        # would succeed.
        set(compile_check_OpenCV TRUE)
    else ()
        try_compile(compile_check_OpenCV
                ${CMAKE_CURRENT_BINARY_DIR}
                SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_CPP_SUBFOLDER}/compileCheckOpenCV/compileCheckOpenCV.cpp
                LINK_LIBRARIES "${OpenCV_LIBS}")
    endif ()

    if (NOT compile_check_OpenCV)
        message("OpenCV example will not be built as the compile check does not compile")
    else ()
        add_executable(sampleOpenCV ${SAMPLE_CPP_SUBFOLDER}/sampleOpenCV/sampleOpenCV.cpp)

        if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
            if (OpenCV_STATIC)
                target_compile_options(sampleOpenCV PRIVATE "/MT$<$<CONFIG:Debug>:d>")
            endif ()
        endif ()

        target_link_libraries(sampleOpenCV royale ${OpenCV_LIBS})

        SET_TARGET_PROPERTIES(sampleOpenCV
                PROPERTIES
                FOLDER samples)
    endif ()
endif ()

# Sample PCL
#------------------------------------------------
option(ROYALE_ENABLE_SAMPLE_PCL "Enable/Disable the PCL example" false)

if (ROYALE_ENABLE_SAMPLE_PCL)
    find_package(PCL 1.3 QUIET)

    if (NOT ${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
        if (PCL_FOUND AND NOT Qt5Widgets_FOUND)
            # The try_compile below will fail with an error that stops the full build, not just the
            # build of the try_compile itself. Caused by https://bugs.debian.org/867454 and 867312
            set(PCL_FOUND False)
            message(WARNING "The Point Cloud Library has been found, but it hasn't added its Qt dependency. If you're running Debian or Ubuntu, please install the libvtk6-qt-dev and libfontconfig1-dev packages.")
        endif ()
    endif ()

    if (NOT PCL_FOUND)
        message("The Point Cloud Library example will not be build as no Point Cloud Library was found (or its dependencies weren't found)")
    else ()
        if (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
            # todo ROYAL-2941 Check compilation on Windows. For now, assume the test compilation
            # would succeed.
            set(compile_check_PCL TRUE)
        else ()
            try_compile(compile_check_PCL
                    ${CMAKE_CURRENT_BINARY_DIR}
                    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_CPP_SUBFOLDER}/compileCheckPCL/compileCheckPCL.cpp
                    LINK_LIBRARIES ${PCL_LIBRARIES})
        endif ()

        if (NOT compile_check_PCL)
            message("Point Cloud Library example will not be built as the compile check does not compile")
        else ()
            include_directories(SYSTEM ${PCL_INCLUDE_DIRS})
            link_directories(${PCL_LIBRARY_DIRS})
            add_definitions(${PCL_DEFINITIONS}
                    -D_SCL_SECURE_NO_WARNINGS
                    -D_CRT_SECURE_NO_WARNINGS)

            add_executable(samplePCL ${SAMPLE_CPP_SUBFOLDER}/samplePCL/samplePCL.cpp)

            target_link_libraries(samplePCL royale ${PCL_LIBRARIES})

            target_compile_options(samplePCL PRIVATE "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:DEBUG>>:/MDd>")

            SET_TARGET_PROPERTIES(samplePCL
                    PROPERTIES
                    FOLDER samples)
        endif ()
    endif ()
endif ()

# Sample OpenNI2Driver
#---------------------------------------------------

find_package(OpenNI2 QUIET)

if (NOT OpenNI2_FOUND)
    message("OpenNI2Driver example will not be build as no OpenNI2 was found!")
else ()

    include_directories(SYSTEM ${OPENNI2_INCLUDE_DIR})

    add_library(royaleONI2 SHARED
            "${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver/RoyaleOpenNI2Driver.cpp"
            "${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver/RoyaleOpenNI2Device.cpp"
            "${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver/RoyaleOpenNI2DepthStream.cpp"
            "${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver/RoyaleOpenNI2Driver.hpp"
            "${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver/RoyaleOpenNI2Device.hpp"
            "${SAMPLE_CPP_SUBFOLDER}/sampleOpenNI2Driver/RoyaleOpenNI2DepthStream.hpp")

    set_target_properties(royaleONI2
            PROPERTIES
            OUTPUT_NAME royaleONI2
            FOLDER modules
            VERSION 0)

    target_link_libraries(royaleONI2 royale ${OPENNI2_LIBRARY})

endif ()
