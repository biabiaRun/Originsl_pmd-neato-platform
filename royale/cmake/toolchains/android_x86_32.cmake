#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

set(ANDROID_TOOLCHAIN_NAME "x86-4.9")
SET(ROYALE_TARGET_PLATFORM "ANDROID" CACHE STRING "" FORCE)
include(${CMAKE_CURRENT_LIST_DIR}/../../contrib/qt-android-cmake-master/toolchain/android.toolchain.cmake)
