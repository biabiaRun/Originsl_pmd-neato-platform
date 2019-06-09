if (NOT ROYALE_TARGET_PLATFORM STREQUAL ANDROID)
    return ()
endif()

set (ANTCOMMAND $ENV{ANT})
if (NOT ANTCOMMAND)
# The environment variable for ANT has to be set
    return ()
endif()    

set (ANDROID_SDK_PATH $ENV{ANDROID_SDK})
if (NOT ANDROID_SDK_PATH)
# The environment variable for the Android SDK has to be set
    return ()
endif()    

set (JAVA_HOME $ENV{JAVA_HOME})
if (NOT JAVA_HOME)
# The environment variable for the Java home folder has to be set
    return ()
endif()

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/java/com/pmdtec/royaleviewer/level2/MainActivity.java" "${CMAKE_CURRENT_BINARY_DIR}/MainActivity.java")

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/AndroidManifest.xml" "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml")

add_custom_target(build_apk ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/app" "${CMAKE_CURRENT_BINARY_DIR}/apk"
  COMMENT "Copying royaleviewerLevel2 sources")

add_custom_command(TARGET build_apk
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/MainActivity.java" "${CMAKE_CURRENT_BINARY_DIR}/apk/src/main/java/com/pmdtec/royaleviewer/level2/MainActivity.java"
  COMMENT "Copying royaleviewerLevel2 MainActivity.java")

add_custom_command(TARGET build_apk
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" "${CMAKE_CURRENT_BINARY_DIR}/apk/AndroidManifest.xml"
  COMMENT "Copying royaleviewerLevel2 AndroidManifest.xml")

add_custom_command(TARGET build_apk
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/res" "${CMAKE_CURRENT_BINARY_DIR}/apk/res"
  COMMENT "Copying royaleviewerLevel2 resources")

add_custom_command(TARGET build_apk
  COMMAND ${ANDROID_SDK_PATH}/tools/android update project -t android-23 --name RoyaleViewerLevel2 --path "${CMAKE_CURRENT_BINARY_DIR}/apk"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/apk")

add_custom_command(TARGET build_apk
  COMMAND ${ANTCOMMAND} release
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/apk")

add_custom_command(TARGET build_apk
  COMMAND ${JAVA_HOME}/bin/jarsigner -verbose -keystore "${ROYALE_SOURCE_DIR}/contrib/qt-android-cmake-master/pmd.keystore" -storepass pmdtechnologies ${CMAKE_CURRENT_BINARY_DIR}/apk/bin/RoyaleViewerLevel2-release-unsigned.apk pmd
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/apk")

add_custom_command(TARGET build_apk
  COMMAND ${ANDROID_SDK_PATH}/build-tools/zipalign -v -f 4 ${CMAKE_CURRENT_BINARY_DIR}/apk/bin/RoyaleViewerLevel2-release-unsigned.apk ${CMAKE_CURRENT_BINARY_DIR}/apk/bin/RoyaleViewerLevel2.apk
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/apk")

