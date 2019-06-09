/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <jni.h>
#include <memory>
#include <cstdint>

#include <royale.hpp>
#include <royale/CameraManager.hpp>
#include <royale/ICameraDevice.hpp>
#include <royale/IDepthDataListener.hpp>
#include <royale/Status.hpp>
#include <royale/String.hpp>

#include "JNIDepthDataListener.hpp"
#include "JNIDepthImageListener.hpp"
#include "JNIEventListener.hpp"
#include "JNIExposureListener.hpp"
#include "JNIIRImageListener.hpp"
#include "JNIRecordStopListener.hpp"
#include "JNISparsePointCloudListener.hpp"
#include "JNICameraDevice.hpp"
#include "JNIHandle.hpp"
#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    extern "C" {

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_initializeNative (JNIEnv *, jobject, jstring);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_finalizeNative (JNIEnv *, jobject);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_getAccessLevel (JNIEnv *, jobject, jstring);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_createCamera__JJJ (JNIEnv *, jobject, jlong, jlong, jlong);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_createCamera__Ljava_lang_String_2 (JNIEnv *, jobject, jstring);

    };
}
