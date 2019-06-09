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
#include <thread>

#include <royale/Status.hpp>
#include <royale/ICameraDevice.hpp>

#include "JavaVM.hpp"
#include "JNIHandle.hpp"

#include "JNIDepthDataListener.hpp"
#include "JNIDepthImageListener.hpp"
#include "JNIEventListener.hpp"
#include "JNIExposureListener.hpp"
#include "JNIIRImageListener.hpp"
#include "JNIRecordStopListener.hpp"
#include "JNISparsePointCloudListener.hpp"

#include "JNILensParameters.hpp"

#define THROW_CAMERA_STATUS_IF_NULL_VOID(MESSAGE)\
    if (CameraStatus::SUCCESS != status)\
    {\
        auto statusString = royale::getStatusString (status);\
        std::string msg = MESSAGE\
                          " CameraStatus=[";\
        msg += statusString.c_str ();\
        msg += ']';\
        env->ThrowNew (jCameraException, msg.c_str ());\
        return;\
    }

#define THROW_CAMERA_STATUS_IF_NULL(MESSAGE)\
    if (CameraStatus::SUCCESS != status)\
    {\
        auto statusString = royale::getStatusString (status);\
        std::string msg = MESSAGE\
                          " CameraStatus=[";\
        msg += statusString.c_str ();\
        msg += ']';\
        env->ThrowNew (jCameraException, msg.c_str ());\
        return NULL;\
    }

#define THROW_CAMERA_STATUS_IF_NULL_JINT(MESSAGE)\
    THROW_CAMERA_STATUS_IF_NULL_(MESSAGE, static_cast<jint>(0))

#define THROW_CAMERA_STATUS_IF_NULL_JBOOL(MESSAGE)\
    THROW_CAMERA_STATUS_IF_NULL_(MESSAGE, static_cast<jboolean>(0))

#define THROW_CAMERA_STATUS_IF_NULL_JLONG(MESSAGE)\
    THROW_CAMERA_STATUS_IF_NULL_(MESSAGE, static_cast<jlong>(0))

#define THROW_CAMERA_STATUS_IF_NULL_(MESSAGE, RETURN)\
    if (CameraStatus::SUCCESS != status)\
    {\
        auto statusString = royale::getStatusString (status);\
        std::string msg = MESSAGE\
                          " CameraStatus=[";\
        msg += statusString.c_str ();\
        msg += ']';\
        env->ThrowNew (jCameraException, msg.c_str ());\
        return RETURN;\
    }

#define RETURN_ON_ERROR(MESSAGE)\
    if (env->ExceptionCheck()) {\
        LOGE(MESSAGE)\
        return NULL;\
    }

#define RETURN_ON_ERROR_VOID(MESSAGE)\
    if (env->ExceptionCheck()) {\
        LOGE(MESSAGE)\
        return;\
    }

namespace jroyale
{

    using namespace royale;

    extern "C" {

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_initialize (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_finalizeNative (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_startCapture (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_stopCapture (JNIEnv *, jobject);

    JNIEXPORT jstring JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getId (JNIEnv *, jobject);

    JNIEXPORT jstring JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getCameraName (JNIEnv *, jobject);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getCameraInfo (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setUseCase (JNIEnv *, jobject, jstring);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getUseCases (JNIEnv *, jobject);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getStreams (JNIEnv *, jobject);

    JNIEXPORT jlong JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getNumberOfStreams (JNIEnv *, jobject, jstring);

    JNIEXPORT jstring JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getCurrentUseCase (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setExposureTime (JNIEnv *, jobject, jlong, jint);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setExposureMode (JNIEnv *, jobject, jobject, jlong);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getExposureMode (JNIEnv *, jobject, jint);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getExposureLimits (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerDataListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterDataListener (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerDepthImageListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterDepthImageListener (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerSparsePointCloudListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterSparsePointCloudListener (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerIRImageListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterIRImageListener (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerEventListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterEventListener (JNIEnv *, jobject);

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getMaxSensorWidth (JNIEnv *, jobject);

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getMaxSensorHeight (JNIEnv *, jobject);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getLensParameters (JNIEnv *, jobject);

    JNIEXPORT jboolean JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_isConnected (JNIEnv *, jobject);

    JNIEXPORT jboolean JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_isCalibrated (JNIEnv *, jobject);

    JNIEXPORT jboolean JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_isCapturing (JNIEnv *, jobject);

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getAccessLevel (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_startRecording (JNIEnv *, jobject, jstring, jlong, jlong, jlong);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_stopRecording (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerRecordListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterRecordListener (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerExposureListener (JNIEnv *, jobject, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterExposureListener (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setFrameRate (JNIEnv *, jobject, jint);

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getFrameRate (JNIEnv *, jobject);

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getMaxFrameRate (JNIEnv *, jobject);

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setExternalTrigger (JNIEnv *, jobject, jboolean);

    };
}
