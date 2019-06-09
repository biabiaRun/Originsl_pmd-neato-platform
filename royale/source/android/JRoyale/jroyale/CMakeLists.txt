# set the required cmake version for this project
cmake_minimum_required( VERSION 3.6 )

# set the language level of C++-Files and C-Files
set( CMAKE_C_STANDARD 99 )
set( CMAKE_CXX_STANDARD 11 )

add_definitions(-DTARGET_PLATFORM_ANDROID)

# set the path to the royale header-Files
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/include" )

# set the path to the royale libraries
link_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}" )

# set the C++-Files to be added to the target library
add_library( jroyale SHARED

             # the header files
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JavaVM.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNICameraDevice.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNICameraManager.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIHandle.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIUtils.hpp"

             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIDepthData.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIDepthImage.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIEvent.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIIRImage.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNILensParameters.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNISparsePointCloud.hpp"

             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIDepthDataListener.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIDepthImageListener.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIEventListener.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIExposureListener.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIIRImageListener.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNIRecordStopListener.hpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/inc/JNISparsePointCloudListener.hpp"

             # the source files
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JavaVM.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNICameraDevice.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNICameraManager.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIHandle.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIUtils.cpp"

             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIDepthData.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIDepthImage.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIEvent.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIIRImage.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNILensParameters.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNISparsePointCloud.cpp"

             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIDepthDataListener.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIDepthImageListener.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIEventListener.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIExposureListener.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIIRImageListener.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNIRecordStopListener.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/JNISparsePointCloudListener.cpp" )

# set the target library to build (jroyale) and it's dependencies to be linked and compiled
target_link_libraries( jroyale

                       # android libraries
                       android
                       log

                       # royale libraries
                       royale
                       spectre
                       usb_android )
