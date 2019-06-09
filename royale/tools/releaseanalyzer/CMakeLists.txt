
cmake_policy(SET CMP0043 NEW)

# On systems that need it, find QT based on the QTDIR environment variable
if (DEFINED ENV{QTDIR})
    set (CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    ADD_DEFINITIONS(-DTARGET_PLATFORM_ANDROID)
ENDIF()

# Widgets finds its own dependencies.
find_package(Qt5Core REQUIRED)
find_package(Qt5XmlPatterns REQUIRED)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/settings.xml.in
    ${CMAKE_CURRENT_SOURCE_DIR}/settings.xml
)

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

#find_package(OpenGL)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/royale/inc
  ${CMAKE_CURRENT_BINARY_DIR}/../../contrib/libziputils
  ${ROYALE_LIBZIPUTILS_DIR}
  ${ROYALE_BZLIB_INCLUDE_DIR}
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/PackageHandler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/CompileHandler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp"
  )

set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/PackageHandler.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/CompileHandler.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/Logger.hpp"
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/PackageHandler.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/CompileHandler.hpp"
  )
  
link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_executable(releaseanalyzer SHARED
      ${SOURCES}
      ${HEADERS}
      ${MOC_HEADERS}
      ${MOCED_HEADERS}
      )
ELSE (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_executable(releaseanalyzer
      ${SOURCES}
      ${HEADERS}
      ${MOC_HEADERS}
      ${MOCED_HEADERS}
      )
ENDIF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
  
target_link_libraries(
  releaseanalyzer
  Qt5::Core
  Qt5::Widgets
  Qt5::XmlPatterns
)

IF (NOT ${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
  target_link_libraries(
    releaseanalyzer
    ${QT_LIBRARIES}
    zipUtils
  )
ELSE()
  target_link_libraries(
    releaseanalyzer
    ${QT_LIBRARIES}
  )
ENDIF()

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})

SET_TARGET_PROPERTIES(releaseanalyzer
    PROPERTIES
    FOLDER tools
    )

IF (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    set(QT_BIN_PATH "${_qt5Widgets_install_prefix}/bin")
	set(QT_PLUGINS_PATH "${_qt5Widgets_install_prefix}/plugins")
ENDIF()

if(ROYALE_DO_VERIFICATION)  
  if (WIN32)
    # Get Qt files
    FILE(GLOB inFiles RELATIVE "${QT_BIN_PATH}/" "${QT_BIN_PATH}/icudt[0-9][0-9].dll" "${QT_BIN_PATH}/icuuc[0-9][0-9].dll" "${QT_BIN_PATH}/icuin[0-9][0-9].dll" "${QT_BIN_PATH}/Qt[0-9]Network.dll" "${QT_BIN_PATH}/Qt[0-9]Core.dll" "${QT_BIN_PATH}/Qt[0-9]Gui.dll" "${QT_BIN_PATH}/Qt[0-9]XmlPatterns.dll" "${QT_BIN_PATH}/Qt[0-9]Widgets.dll")
    FOREACH(infileName ${inFiles})
      ADD_CUSTOM_COMMAND(TARGET releaseanalyzer
        POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy "${QT_BIN_PATH}/${infileName}" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/${infileName}"
      )
    ENDFOREACH(infileName)
	
    ADD_CUSTOM_COMMAND(TARGET releaseanalyzer
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:releaseanalyzer>" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/$<TARGET_FILE_NAME:releaseanalyzer>"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_LIST_DIR}/settings.xsd" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/settings.xsd" 
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_LIST_DIR}/settings.xml" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/settings.xml" 
    )
  else()
    ADD_CUSTOM_COMMAND(TARGET releaseanalyzer
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:releaseanalyzer>" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/$<TARGET_FILE_NAME:releaseanalyzer>"
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:zipUtils>" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/$<TARGET_FILE_NAME:zipUtils>"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_LIST_DIR}/settings.xsd" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/settings.xsd" 
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_LIST_DIR}/settings.xml" "${ROYALE_SOURCE_DIR}/testing/releaseanalyzer/bin/settings.xml" 
    )
  endif()
endif()
