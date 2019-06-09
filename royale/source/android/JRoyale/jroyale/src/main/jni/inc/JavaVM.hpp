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
#include <android/log.h>

#define TAG "com.pmdtec.jroyale.jni"

#define LOGA(...)  __android_log_print(ANDROID_LOG_ASSERT, TAG, __VA_ARGS__);
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);
#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__);
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, TAG,__VA_ARGS__);

/*
 * this file provides some global lookups an macros
 * its also contains the onLoad an onUnload Function of jroyale
 */
namespace jroyale
{
    extern "C" {

    JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM *vm, void *reserved);
    JNIEXPORT void JNICALL JNI_OnUnload (JavaVM *vm, void *reserved);

    }

    // initialize java

    extern JavaVM *javaVM;

    extern jclass jArrayList;
    extern jmethodID jArrayList_init;
    extern jmethodID jArrayList_add;

    extern jclass jHashMap;
    extern jmethodID jHashMap_init;
    extern jmethodID jHashMap_put;

    extern jclass jBoolean;
    extern jmethodID jBoolean_init;

    extern jclass jByte;
    extern jmethodID jByte_init;

    extern jclass jCharacter;
    extern jmethodID jCharacter_init;

    extern jclass jShort;
    extern jmethodID jShort_init;

    extern jclass jInteger;
    extern jmethodID jInteger_init;

    extern jclass jLong;
    extern jmethodID jLong_init;

    extern jclass jFloat;
    extern jmethodID jFloat_init;

    extern jclass jDouble;
    extern jmethodID jDouble_init;

    // initialize android

    extern jclass jPair;
    extern jmethodID jPair_init;
    extern jfieldID jPair_first;
    extern jfieldID jPair_second;

    // initialize com.pmdtec.jroyale

    extern jclass jCameraException;

    extern jclass jRoyaleCameraManager;

    extern jclass jRoyaleCameraDevice;
    extern jmethodID jRoyaleCameraDevice_init;

    extern jclass jCameraAccessLevel;
    extern jobject jCameraAccessLevel_L1;
    extern jobject jCameraAccessLevel_L2;
    extern jobject jCameraAccessLevel_L3;
    extern jobject jCameraAccessLevel_L4;

    extern jclass jCamerEvent$Severity;
    extern jobject jEvent$Severity_INFO;
    extern jobject jEvent$Severity_WARNING;
    extern jobject jEvent$Severity_ERROR;
    extern jobject jEvent$Severity_FATAL;

    extern jclass jCamerEvent$Type;
    extern jobject jEvent$Type_CAPTURE_STREAM;
    extern jobject jEvent$Type_DEVICE_DISCONNECTED;
    extern jobject jEvent$Type_OVER_TEMPERATURE;
    extern jobject jEvent$Type_RAW_FRAME_STATS;

    extern jclass jExposureMode;
    extern jobject jExposureMode_MANUAL;
    extern jobject jExposureMode_AUTOMATIC;

    // initialize handle references

    extern jfieldID jRoyaleCameraManager_mHandle;

    extern jfieldID jRoyaleCameraDevice_mHandle;
    extern jfieldID jRoyaleCameraDevice_mDepthDataListenerHandle;
    extern jfieldID jRoyaleCameraDevice_mDepthImageListenerHandle;
    extern jfieldID jRoyaleCameraDevice_mEventListenerHandle;
    extern jfieldID jRoyaleCameraDevice_mExposureListenerHandle;
    extern jfieldID jRoyaleCameraDevice_mIRImageListenerHandle;
    extern jfieldID jRoyaleCameraDevice_mRecordStopListenerHandle;
    extern jfieldID jRoyaleCameraDevice_mSparsePointCloudListenerHandle;

    // initialize com.pmdtec.jroyale.data

    extern jclass jDepthData;
    extern jmethodID jDepthData_init;

    extern jclass jDepthPoint;
    extern jmethodID jDepthPoint_init;

    extern jclass jDepthImage;
    extern jmethodID jDepthImage_init;

    extern jclass jCameraEvent;
    extern jmethodID jEvent_createEvent;

    extern jclass jIRImage;
    extern jmethodID jIRImage_init;

    extern jclass jLensParameters;
    extern jmethodID jLensParameters_init;

    extern jclass jSparsePointCloud;
    extern jmethodID jSparsePointCloud_init;

    // initialize com.pmdtec.jroyale.listener

    extern jclass jDepthDataListener;
    extern jmethodID jDepthDataListener_onNewData;

    extern jclass jDepthImageListener;
    extern jmethodID jDepthImageListener_onNewData;

    extern jclass jCameraEventListener;
    extern jmethodID jCameraEventListener_onEvent;

    extern jclass jExposureListener;
    extern jmethodID jExposureListener_onNewExposure;

    extern jclass jIRImageListener;
    extern jmethodID jIRImageListener_onNewData;

    extern jclass jRecordStopListener;
    extern jmethodID jRecordStopListener_onRecordingStopped;

    extern jclass jSparsePointCloudListener;
    extern jmethodID jSparsePointCloudListener_onNewData;
}
