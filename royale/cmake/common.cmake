#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(ROYALE_INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
SET(ROYALE_INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
SET(ROYALE_INSTALL_DOT_NET_DIR dot_net CACHE PATH "Installation directory for .NET wrapper")
SET(ROYALE_INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
SET(ROYALE_INSTALL_DOC_DIR doc CACHE PATH "Installation directory for header files")
SET(ROYALE_INSTALL_DRIVER_DIR driver CACHE PATH "Installation directory for drivers")
SET(ROYALE_INSTALL_SAMPLES_DIR samples CACHE PATH "Installation directory for samples")
SET(ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_DIR        "${ROYALE_INSTALL_SAMPLES_DIR}/android/sampleNativeAndroid/app/src/main/jniLibs/${ANDROID_ABI}/"         CACHE STRING "" FORCE)
SET(ROYALE_INSTALL_ANDROID_NATIVE_SAMPLE_LIB_HEADER_DIR "${ROYALE_INSTALL_SAMPLES_DIR}/android/sampleNativeAndroid/app/src/main/jniLibs/${ANDROID_ABI}/include/" CACHE STRING "" FORCE)
SET(ROYALE_INSTALL_ROYALECONFIG_DIR share CACHE PATH "Installation directory for royale-config.cmake")
SET(ROYALE_INSTALL_MATLAB_DIR "matlab/+royale" CACHE PATH "Installation directory for MATLAB wrapper")
SET(ROYALE_INSTALL_MATLAB_EXAMPLE_DIR matlab CACHE PATH "Installation directory for MATLAB wrapper")
SET(ROYALE_INSTALL_PYTHON_DIR python CACHE PATH "Installation directory for PYTHON wrapper")

FUNCTION(ROYALE_SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            SET(dirlist ${dirlist} ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(ROYALE_ADD_SUBDIRECTORY dir)
    SET(dirabsolute "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
    IF( EXISTS "${dirabsolute}/CMakeLists.txt")
        ADD_SUBDIRECTORY( "${dir}" )
    ELSE()
#        MESSAGE("cannot add directory:${dirintern}")
    ENDIF()
ENDFUNCTION()

FUNCTION(ROYALE_INCLUDE_ALL_SUBDIRS)
    ROYALE_SUBDIRLIST( incdirs ${CMAKE_CURRENT_SOURCE_DIR} )
    LIST( SORT incdirs )
    FOREACH( incdir ${incdirs} )
        ROYALE_ADD_SUBDIRECTORY("${incdir}")
    ENDFOREACH()
ENDFUNCTION()

FUNCTION(ROYALE_REMOVE_FROM_STRING stringname value)
    IF(DEFINED "${stringname}")
        IF(NOT "${${stringname}}" STREQUAL "")
            STRING( REGEX REPLACE "${value}" "" "${stringname}" "${${stringname}}" )
            SET("${stringname}" "${${stringname}}" PARENT_SCOPE)
        ELSE()
            MESSAGE("string with name ${stringname} is empty!")
        ENDIF()
    ELSE()
        MESSAGE("no string with name ${stringname}")
    ENDIF()
ENDFUNCTION()

macro(ROYALE_INCLUDE_TARGET_FILE)
    if (${ARGC} GREATER 0)
        set(SEARCHDIR "${ARGV0}")
    else ()
        set(SEARCHDIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif ()
    if (ROYALE_TARGET_PLATFORM)
        string(TOLOWER ${ROYALE_TARGET_PLATFORM} RTP)
        set(targetfile "${SEARCHDIR}/${RTP}.cmake")
        if (EXISTS "${targetfile}")
            include(${targetfile})
        endif ()
    endif ()
endmacro()

SET(CMAKE_MODULE_PATH "${ROYALE_SOURCE_DIR}/cmake/Modules")
find_package(Pandoc COMPONENTS LATEX_ENGINE_OPT)


FUNCTION(CREATE_PDF_FROM_MARKDOWN filename title subtitle version confidential outputfile)
    IF(PANDOC_FOUND AND ROYALE_ENABLE_PANDOC)
        GET_FILENAME_COMPONENT(MD_FILE ${filename} NAME_WE)
        SET(ROYALE_PANDOC_PATH "${ROYALE_SOURCE_DIR}/doc/pandoc")
        SET(PANDOC_OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/pandoc_${MD_FILE}")
        SET(PANDOC_IMG_OUT_PATH "${ROYALE_BINARY_DIR}/pandoc")
        SET(ROYALE_PANDOC_IMG_PATH "${PANDOC_IMG_OUT_PATH}/img")
        SET(ROYALE_PANDOC_TITLE ${title})
        SET(ROYALE_PANDOC_SUBTITLE ${subtitle})
        SET(ROYALE_PANDOC_VERSION ${version})
        SET(ROYALE_CONFIDENTIAL_LEVEL ${confidential})
        CONFIGURE_FILE("${ROYALE_PANDOC_PATH}/pandoc_template.tex" "${PANDOC_OUT_PATH}/pandoc_template.tex" @ONLY)
        CONFIGURE_FILE("${ROYALE_PANDOC_PATH}/pmd.sty" "${PANDOC_OUT_PATH}/pmd.sty" @ONLY)
        FILE(COPY "${ROYALE_PANDOC_PATH}/img" DESTINATION "${PANDOC_IMG_OUT_PATH}")
        SET(outputfile "${PANDOC_OUT_PATH}/${MD_FILE}.pdf" PARENT_SCOPE)

        ADD_CUSTOM_COMMAND(
            OUTPUT "${PANDOC_OUT_PATH}/${filename}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${filename}" "${PANDOC_OUT_PATH}/${filename}"
            MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${filename}"
            )
        #FILE(COPY "${filename}" DESTINATION "${PANDOC_OUT_PATH}")
        ADD_CUSTOM_COMMAND(
            COMMAND ${PANDOC_EXECUTABLE}
            ARGS --latex-engine-opt=-shell-escape --listings --template=pandoc_template.tex ${filename} -o ${MD_FILE}.pdf
            WORKING_DIRECTORY "${PANDOC_OUT_PATH}"
            DEPENDS "${PANDOC_OUT_PATH}/${filename}"
            OUTPUT "${PANDOC_OUT_PATH}/${MD_FILE}.pdf"
            )

        ADD_CUSTOM_TARGET(Pandoc_${MD_FILE} ALL
            DEPENDS "${PANDOC_OUT_PATH}/${filename}" "${PANDOC_OUT_PATH}/${MD_FILE}.pdf"
            )
        SET_TARGET_PROPERTIES(Pandoc_${MD_FILE}
           PROPERTIES
           FOLDER Pandoc_Documentation
           )
    ENDIF()
ENDFUNCTION()

FUNCTION(INSTALL_MARKDOWN_FILE filename title subtitle version confidential installfolder)
    IF(PANDOC_FOUND AND ROYALE_ENABLE_PANDOC)
        CREATE_PDF_FROM_MARKDOWN (${filename} ${title} ${subtitle} ${version} ${confidential} outputfile)
        INSTALL(FILES ${outputfile} DESTINATION "${installfolder}")
    ELSE()
        INSTALL(FILES "${filename}" DESTINATION "${installfolder}")
    ENDIF()
ENDFUNCTION()

SET(ROYALE_COVERAGE_GCOVR "OFF" CACHE STRING "If test coverage build is enabled, by default a lcov code coverage report will be produced, setting this flag to ON a gcovr code coverage report will be created")

macro( ROYALE_FIND_HOST_PACKAGE )
    IF(CMAKE_CROSSCOMPILING)
        set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
        set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
        set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )
        IF( CMAKE_HOST_WIN32 )
            SET( WIN32 1 )
            SET( UNIX )
        ELSEIF( CMAKE_HOST_APPLE )
            SET( APPLE 1 )
            SET( UNIX )
        ENDIF()
        find_package( ${ARGN} )
        SET( WIN32 )
        SET( APPLE )
        SET( UNIX 1 )
        set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
        set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
        set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
    ELSE(CMAKE_CROSSCOMPILING)
        find_package( ${ARGN} )
    ENDIF()
endmacro()
