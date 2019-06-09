#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

set(ANDROID_TOOLCHAIN_NAME "aarch64-linux-android-4.9")
SET(ROYALE_TARGET_PLATFORM "ANDROID" CACHE STRING "" FORCE)

# disable royale parts that can or should not be build on Android 64
OPTION(ROYALE_ENABLE_TOOLS "Build all internal tools" OFF)
OPTION(ROYALE_ENABLE_PANDOC
  "Use Pandoc for md documents (Pandoc has to be installed and in the path)"
  ON
)
include(${CMAKE_CURRENT_LIST_DIR}/../../contrib/qt-android-cmake-master/toolchain/android.toolchain.cmake)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
