# On systems that need it, find QT based on the QTDIR environment variable
if (DEFINED ENV{QTDIR})
    set (CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

OPTION(ROYALE_VIEWER_CONSOLE "Enable console for royaleviewer" OFF)

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    ADD_DEFINITIONS(-DTARGET_PLATFORM_ANDROID)
    ADD_DEFINITIONS(-DELPP_DEFAULT_LOG_FILE="${ROYALE_ANDROID_LOG_FILE}")
    ADD_DEFINITIONS(-DELPP_QT_LOGGING)
ENDIF()

# Widgets finds its own dependencies.
find_package(Qt5Widgets 5.5 REQUIRED)
find_package(Qt5OpenGL REQUIRED)

find_package(OpenGL)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc/common      # includes which have moved to core already
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc             # includes which have moved to core already
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/2DView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/3DView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ArcBall.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Axis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ColorHelper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Depth3d.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Frustum.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Math0.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/PLYWriter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/PixelInfoManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Renderable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/AutoRotationView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/CameraPositionView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ColorRangeView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/DataSelectorView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/DisplaySupport.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ExposureView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterLevelView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterMinMaxView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FramerateView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/HelpView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/LicenseView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ListView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/OpenFileView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ParameterView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDButton.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDStreamView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PixelInfoView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PlayerControl.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/RegisterView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/SettingsMenuView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/StartupMessageView.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/qtviewer.cpp"
  )

set(HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/ArcBall.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/Axis.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/ColorHelper.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/Depth3d.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/FileHelper.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/Frustum.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/Math0.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/MutexTryLocker.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/PLYWriter.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/PixelInfoManager.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/Renderable.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/AutoRotationView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/CameraPositionView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ColorRangeView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/DataSelectorView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/DisplaySupport.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ExposureView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterLevelView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterMinMaxView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FramerateView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/HelpView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/LicenseView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ListView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/OpenFileView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ParameterView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDButton.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDStreamView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PixelInfoView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PlayerControl.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/RegisterView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/SettingsMenuView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/StartupMessageView.hpp"
  )

set(MOC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/2DView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/3DView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/IView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/qtviewer.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/AutoRotationView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/CameraPositionView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ColorRangeView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/DataSelectorView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ExposureView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterLevelView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterMinMaxView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FramerateView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/HelpView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/LicenseView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ListView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/OpenFileView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ParameterView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDStreamView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PMDView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PixelInfoView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PlayerControl.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/RegisterView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/SettingsMenuView.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/StartupMessageView.hpp"
  )

set(UI_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/AutoRotationView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/CameraPositionView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ColorRangeView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/DataSelectorView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ExposureView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterLevelView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FilterMinMaxView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/FramerateView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/HelpView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/LicenseView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ListView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/OpenFileView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/ParameterView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PixelInfoView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/PlayerControl.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/RegisterView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/SettingsMenuView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/custom-controls/StartupMessageView.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui"
  )

set(JAVA_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/javasrc/src/org/pmdtec/qtviewer/ProbingActivity.java"
  )


set(ROYALE_S7_WORKAROUND ON CACHE BOOL "Work around Samsung Galaxy S7 point rendering issue")

if(${ROYALE_S7_WORKAROUND})
  ADD_DEFINITIONS(-DROYALE_S7_WORKAROUND)
  set(HEADERS ${HEADERS} "${CMAKE_CURRENT_SOURCE_DIR}/inc/Depth3dSamsungS7.hpp")
  set(SOURCES ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/Depth3dSamsungS7.cpp")
endif(${ROYALE_S7_WORKAROUND})



link_directories(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOCED_HEADERS ${MOC_HEADERS})

set(ROYALE_VIEWER_LICENSES "<file alias=\"royale_license\">${ROYALE_LICENSE_PATH}</file>\n")
if(${ROYALE_PLAYBACK_ONLY})
  set(LICENSE_LIST "gradle-wrapper_license;jquery_license;kissfft_license;packedarray_license;sizzle_license;gpl-3.0;lgpl-2.1;lgpl-3.0")
else()
  set(LICENSE_LIST "cypress_license;gradle-wrapper_license;jquery_license;kissfft_license;packedarray_license;sizzle_license;gpl-3.0;lgpl-2.1;lgpl-3.0")
endif()

foreach(curlicense ${LICENSE_LIST})
  string(CONCAT ROYALE_VIEWER_LICENSES ${ROYALE_VIEWER_LICENSES} "<file alias=\"${curlicense}\">${ROYALE_SOURCE_DIR}/doc/licenses/${curlicense}.txt</file>\n")
endforeach()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/assets.qrc.in" "${CMAKE_CURRENT_BINARY_DIR}/assets.qrc")

qt5_add_resources(
  RESOURCES
  "${CMAKE_CURRENT_BINARY_DIR}/assets.qrc"
  )

set(RC_PRODUCTNAME "royaleviewer")
set(RC_FILETYPE "VFT_APP")
set(RC_FILEDESCRIPTION "Viewer for Royale")
set(RC_INTERNALNAME "royaleviewer")
set(RC_ORIGINALFILENAME "royaleviewer")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)

SET(WIN32_RESSOURCE_FILES "")
IF (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    SET(WIN32_RESSOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/misc/royaleviewer.rc"
        )
ENDIF()


set(CMAKE_OSX_BUNDLE_FLAG "")
set(OSXICON "")
IF (${ROYALE_TARGET_PLATFORM} STREQUAL APPLE)
    SET(CMAKE_OSX_BUNDLE_FLAG "MACOSX_BUNDLE")
    SET(MACOSX_BUNDLE_INFO_STRING "royaleviewer ${ROYALE_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "royaleviewer ${ROYALE_VERSION}")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "royaleviewer ${ROYALE_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${ROYALE_VERSION}")
    SET(MACOSX_BUNDLE_COPYRIGHT "pmdtechnologies ag")
    SET(MACOSX_BUNDLE_ICON_FILE "pmdlogo.icns")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "royaleviewer ${ROYALE_VERSION}")
    SET(MACOSX_BUNDLE_RESOURCES "${ROYALE_INSTALL_BIN_DIR}/royaleviewer.app/Contents/Resources")
    SET(MACOSX_BUNDLE_ICON "${MACOSX_BUNDLE_RESOURCES}/${MACOSX_BUNDLE_ICON_FILE}")

    set(OSXICON ${CMAKE_CURRENT_SOURCE_DIR}/misc/pmdlogo.icns)
    set_source_files_properties(${OSXICON} PROPERTIES
           MACOSX_PACKAGE_LOCATION "Resources")
ENDIF()

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    add_library(royaleviewer SHARED
      ${SOURCES}
      ${HEADERS}
      ${MOC_HEADERS}
      ${UI_HEADERS}
      ${MOCED_HEADERS}
      ${RESOURCES}
      )
ELSE (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    add_executable(royaleviewer
      ${CMAKE_OSX_BUNDLE_FLAG}
      ${OSXICON}
      ${SOURCES}
      ${HEADERS}
      ${MOC_HEADERS}
      ${UI_HEADERS}
      ${MOCED_HEADERS}
      ${RESOURCES}
      ${ASSEMBLY_RC}
      ${WIN32_RESSOURCE_FILES}
      )
ENDIF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)

IF (${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    SET(ROYALE_VIEWER_WIN32_EXECUTABLE ON)
    IF(${ROYALE_VIEWER_CONSOLE})
        SET(ROYALE_VIEWER_WIN32_EXECUTABLE OFF)
    ENDIF()
    SET_TARGET_PROPERTIES( royaleviewer
        PROPERTIES
        WIN32_EXECUTABLE ${ROYALE_VIEWER_WIN32_EXECUTABLE}
        )
ENDIF()

IF (${ROYALE_TARGET_PLATFORM} STREQUAL APPLE)
    # Use a different plist template
    set_target_properties(royaleviewer PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${ROYALE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in")
ENDIF()

include_directories( SYSTEM
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    )

target_link_libraries(royaleviewer Qt5::Widgets Qt5::OpenGL)

target_link_libraries(royaleviewer royale ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    find_package(Qt5AndroidExtras REQUIRED)

    target_link_libraries(royaleviewer Qt5::AndroidExtras)
    target_link_libraries(royaleviewer GLESv1_CM log)
ENDIF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)

SOURCE_GROUP( "inc" FILES ${HEADERS} ${MOC_HEADERS})
SOURCE_GROUP( "src" FILES ${SOURCES})
SOURCE_GROUP( "java" FILES ${JAVA_SOURCES})

SET_TARGET_PROPERTIES(royaleviewer
    PROPERTIES
    FOLDER tools
    )

IF (${ROYALE_TARGET_PLATFORM} STREQUAL ANDROID)
    # We need a unique number which is increased for every release 
    # This assumes that we use 2 numbers each for major, minor and patch
    math(EXPR ROYALE_ANDROID_VERSION_CODE_MAJOR "${ROYALE_VERSION_MAJOR} * 10000")
    math(EXPR ROYALE_ANDROID_VERSION_CODE_MINOR "${ROYALE_VERSION_MINOR} * 100")
    math(EXPR ROYALE_ANDROID_VERSION_CODE_PATCH "${ROYALE_VERSION_PATCH} * 1")
    math(EXPR ROYALE_ANDROID_VERSION_CODE "${ROYALE_ANDROID_VERSION_CODE_MAJOR} + ${ROYALE_ANDROID_VERSION_CODE_MINOR} + ${ROYALE_ANDROID_VERSION_CODE_PATCH}")

    if(ROYALE_USE_SPECTRE)
        set(PROCESSING_LIB spectre)
    else()
        set(PROCESSING_LIB "")
    endif()

    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/javasrc" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/javasrc/AndroidManifest.xml" "${CMAKE_CURRENT_BINARY_DIR}/javasrc/AndroidManifest.xml")

    set_property(TARGET royaleviewer PROPERTY VERSION ${ROYALE_VERSION})
    include(../../contrib/qt-android-cmake-master/AddQtAndroidApk.cmake)
    add_qt_android_apk(royaleviewer_apk royaleviewer
        NAME "royaleviewer ${ROYALE_VERSION}"
        PACKAGE_NAME "org.pmdtec.qtviewer"
        PACKAGE_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/javasrc"
        DEPENDS royale ${ROYALE_USB_LINK_COMMAND} ${PROCESSING_LIB}
        KEYSTORE "${CMAKE_CURRENT_SOURCE_DIR}/../../contrib/qt-android-cmake-master/pmd.keystore" pmd
        KEYSTORE_PASSWORD pmdtechnologies
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/QtApp-release-signed.apk" DESTINATION ${ROYALE_INSTALL_BIN_DIR} RENAME RoyaleViewer.apk)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/README_Android.md" DESTINATION ${ROYALE_INSTALL_BIN_DIR} RENAME README.md)
ELSEIF (${ROYALE_TARGET_PLATFORM} STREQUAL APPLE)
    INCLUDE("${ROYALE_SOURCE_DIR}/cmake/DeployQt5.cmake")
    install(TARGETS royaleviewer
        BUNDLE DESTINATION ${ROYALE_INSTALL_BIN_DIR})
    install_qt5_executable("${ROYALE_INSTALL_BIN_DIR}/royaleviewer.app" "" "" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${CMAKE_INSTALL_PREFIX}/${ROYALE_INSTALL_BIN_DIR};${_qt5Widgets_install_prefix}/lib;${PROCESSING_SPECTRE_SUBFOLDER}" "" "")
ELSEIF(${ROYALE_TARGET_PLATFORM} STREQUAL WINDOWS)
    INCLUDE("${ROYALE_SOURCE_DIR}/cmake/DeployQt5.cmake")
    install(TARGETS royaleviewer
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})
    install_qt5_executable("${ROYALE_INSTALL_BIN_DIR}/royaleviewer.exe" "" "" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${_qt5Widgets_install_prefix}/bin;${PROCESSING_SPECTRE_SUBFOLDER}" "" "")
ELSE () #LINUX
    INCLUDE("${ROYALE_SOURCE_DIR}/cmake/DeployQt5.cmake")
    install(TARGETS royaleviewer
        RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR})
    install_qt5_executable("${ROYALE_INSTALL_BIN_DIR}/royaleviewer" "" "" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${_qt5Widgets_install_prefix}/lib;${PROCESSING_SPECTRE_SUBFOLDER}" "" "")

    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/README_Linux.md" DESTINATION ${ROYALE_INSTALL_BIN_DIR} RENAME README.md)
    install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/misc/royaleviewer.sh" DESTINATION ${ROYALE_INSTALL_BIN_DIR})
ENDIF()
