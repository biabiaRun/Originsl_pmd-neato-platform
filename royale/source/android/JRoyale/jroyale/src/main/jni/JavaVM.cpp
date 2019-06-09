/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JavaVM.hpp"

namespace jroyale
{

// first define some macros to prevent code duplicates
//

#define FIND_CLASS(clazz, path)\
{\
    jclass tmpClazz = env->FindClass (path);\
    if (env->ExceptionCheck ())\
    {\
    LOGE("can not find class=["\
         #clazz\
         "] with path=["\
         path\
         "]");\
    return -1;\
    }\
    LOGI("found class=["\
         #clazz\
         "] with path=["\
         path\
         "]")\
    clazz = reinterpret_cast<jclass>(env->NewGlobalRef (tmpClazz));\
    env->DeleteLocalRef (tmpClazz);\
}

#define GET_METHOD(clazz, method, name, signature)\
{\
    method = env->GetMethodID (clazz, name, signature);\
    if (env->ExceptionCheck ())\
    {\
        LOGE("can not get method=["\
         name\
         "] with signature=["\
         signature\
         "] from class=["\
         #clazz\
         "]")\
        return -1;\
    }\
    LOGI("got method=["\
         name\
         "] with signature=["\
         signature\
         "] from class=["\
         #clazz\
         "]")\
}

#define GET_STATIC_METHOD(clazz, method, name, signature)\
{\
    method = env->GetStaticMethodID (clazz, name, signature);\
    if (env->ExceptionCheck ())\
    {\
        LOGE("can not get static method=["\
         name\
         "] with signature=["\
         signature\
         "] from class=["\
         #clazz\
         "]")\
        return -1;\
    }\
    LOGI("got static method=["\
         name\
         "] with signature=["\
         signature\
         "] from class=["\
         #clazz\
         "]")\
}

#define GET_FIELD(clazz, field, name, signature)\
{\
    field = env->GetFieldID (clazz, name, signature);\
    if (env->ExceptionCheck ())\
    {\
        LOGE("can not get field=["\
        name\
        "] from class=["\
        #clazz\
        "] and with signature=["\
        signature\
        "]")\
        return -1;\
    }\
    LOGI("got field=["\
        name\
        "] from class=["\
        #clazz\
        "] and with signature=["\
        signature\
        "]")\
}

#define GET_ENUM_CONSTANT(clazz, object, name, signature)\
{\
    jfieldID tmpFieldID = env->GetStaticFieldID (clazz, name, signature);\
    if (env->ExceptionCheck ())\
    {\
        LOGE("can not get enum constant=["\
        name\
        "] from class=["\
        #clazz\
        "] and with signature=["\
        signature\
        "] enums field id is not available")\
        return -1;\
    }\
    jobject tmpObject = env->GetStaticObjectField (clazz, tmpFieldID);\
    if (env->ExceptionCheck ())\
    {\
        LOGE("can not get enum constant=["\
        name\
        "] from class=["\
        #clazz\
        "] and with signature=["\
        signature\
        "] enums field is not available")\
        return -1;\
    }\
    LOGI("got enum constant=["\
        name\
        "] from class=["\
        #clazz\
        "] and with signature=["\
        signature\
        "]")\
    object = env->NewGlobalRef (tmpObject);\
    env->DeleteLocalRef (tmpObject);\
}

    // initialize java

    JavaVM *javaVM;

    jclass jArrayList;
    jmethodID jArrayList_init;
    jmethodID jArrayList_add;

    jclass jHashMap;
    jmethodID jHashMap_init;
    jmethodID jHashMap_put;

    jclass jBoolean;
    jmethodID jBoolean_init;

    jclass jByte;
    jmethodID jByte_init;

    jclass jCharacter;
    jmethodID jCharacter_init;

    jclass jShort;
    jmethodID jShort_init;

    jclass jInteger;
    jmethodID jInteger_init;

    jclass jLong;
    jmethodID jLong_init;

    jclass jFloat;
    jmethodID jFloat_init;

    jclass jDouble;
    jmethodID jDouble_init;

    // initialize android

    jclass jPair;
    jmethodID jPair_init;
    jfieldID jPair_first;
    jfieldID jPair_second;

    // initialize com.pmdtec.jroyale

    jclass jCameraException;

    jclass jRoyaleCameraManager;

    jclass jRoyaleCameraDevice;
    jmethodID jRoyaleCameraDevice_init;

    jclass jCameraAccessLevel;
    jobject jCameraAccessLevel_L1;
    jobject jCameraAccessLevel_L2;
    jobject jCameraAccessLevel_L3;
    jobject jCameraAccessLevel_L4;

    jclass jExposureMode;
    jobject jExposureMode_MANUAL;
    jobject jExposureMode_AUTOMATIC;

    jclass jCamerEvent$Severity;
    jobject jEvent$Severity_INFO;
    jobject jEvent$Severity_WARNING;
    jobject jEvent$Severity_ERROR;
    jobject jEvent$Severity_FATAL;

    jclass jCamerEvent$Type;
    jobject jEvent$Type_CAPTURE_STREAM;
    jobject jEvent$Type_DEVICE_DISCONNECTED;
    jobject jEvent$Type_OVER_TEMPERATURE;
    jobject jEvent$Type_RAW_FRAME_STATS;

    // initialize handle references

    jfieldID jRoyaleCameraManager_mHandle;

    jfieldID jRoyaleCameraDevice_mHandle;
    jfieldID jRoyaleCameraDevice_mDepthDataListenerHandle;
    jfieldID jRoyaleCameraDevice_mDepthImageListenerHandle;
    jfieldID jRoyaleCameraDevice_mEventListenerHandle;
    jfieldID jRoyaleCameraDevice_mExposureListenerHandle;
    jfieldID jRoyaleCameraDevice_mIRImageListenerHandle;
    jfieldID jRoyaleCameraDevice_mRecordStopListenerHandle;
    jfieldID jRoyaleCameraDevice_mSparsePointCloudListenerHandle;

    // initialize com.pmdtec.jroyale.data

    jclass jDepthData;
    jmethodID jDepthData_init;

    jclass jDepthPoint;
    jmethodID jDepthPoint_init;

    jclass jDepthImage;
    jmethodID jDepthImage_init;

    jclass jCameraEvent;
    jmethodID jEvent_createEvent;

    jclass jIRImage;
    jmethodID jIRImage_init;

    jclass jLensParameters;
    jmethodID jLensParameters_init;

    jclass jSparsePointCloud;
    jmethodID jSparsePointCloud_init;

    // initialize com.pmdtec.jroyale.listener

    jclass jDepthDataListener;
    jmethodID jDepthDataListener_onNewData;

    jclass jDepthImageListener;
    jmethodID jDepthImageListener_onNewData;

    jclass jCameraEventListener;
    jmethodID jCameraEventListener_onEvent;

    jclass jExposureListener;
    jmethodID jExposureListener_onNewExposure;

    jclass jIRImageListener;
    jmethodID jIRImageListener_onNewData;

    jclass jRecordStopListener;
    jmethodID jRecordStopListener_onRecordingStopped;

    jclass jSparsePointCloudListener;
    jmethodID jSparsePointCloudListener_onNewData;

    extern "C" {

    // initialize the global values
    // for common classes and methods
    // ------------------------------
    //
    JNIEXPORT jint JNICALL
    JNI_OnLoad (JavaVM *vm, void *)
    {
        // initialize java

        javaVM = vm;
        JNIEnv *env;
        if (javaVM->GetEnv (reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
        {
            LOGE("can not cache the java native interface environment");
            return -1;
        }

        // initialize java

        FIND_CLASS (jArrayList, "java/util/ArrayList")
        GET_METHOD (jArrayList, jArrayList_init, "<init>", "(I)V")
        GET_METHOD (jArrayList, jArrayList_add, "add", "(Ljava/lang/Object;)Z")

        FIND_CLASS (jHashMap, "java/util/HashMap")
        GET_METHOD (jHashMap, jHashMap_init, "<init>", "(I)V")
        GET_METHOD (jHashMap, jHashMap_put, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;")

        FIND_CLASS (jBoolean, "java/lang/Boolean")
        GET_METHOD (jBoolean, jBoolean_init, "<init>", "(Z)V")

        FIND_CLASS (jByte, "java/lang/Byte")
        GET_METHOD (jByte, jByte_init, "<init>", "(B)V")

        FIND_CLASS (jCharacter, "java/lang/Character")
        GET_METHOD (jCharacter, jCharacter_init, "<init>", "(C)V")

        FIND_CLASS (jShort, "java/lang/Short")
        GET_METHOD (jShort, jShort_init, "<init>", "(S)V")

        FIND_CLASS (jInteger, "java/lang/Integer")
        GET_METHOD (jInteger, jInteger_init, "<init>", "(I)V")

        FIND_CLASS (jLong, "java/lang/Long")
        GET_METHOD (jLong, jLong_init, "<init>", "(J)V")

        FIND_CLASS (jFloat, "java/lang/Float")
        GET_METHOD (jFloat, jFloat_init, "<init>", "(F)V")

        FIND_CLASS (jDouble, "java/lang/Double")
        GET_METHOD (jDouble, jDouble_init, "<init>", "(D)V")

        // initialize android

        FIND_CLASS (jPair, "android/util/Pair")
        GET_METHOD (jPair, jPair_init, "<init>", "(Ljava/lang/Object;Ljava/lang/Object;)V")
        GET_FIELD (jPair, jPair_first, "first", "Ljava/lang/Object;");
        GET_FIELD (jPair, jPair_second, "second", "Ljava/lang/Object;");

        // initialize com.pmdtec.jroyale

        FIND_CLASS (jCameraException, "com/pmdtec/jroyale/CameraException")

        FIND_CLASS (jRoyaleCameraManager, "com/pmdtec/jroyale/RoyaleCameraManager")

        FIND_CLASS (jRoyaleCameraDevice, "com/pmdtec/jroyale/RoyaleCameraDevice")
        GET_METHOD (jRoyaleCameraDevice, jRoyaleCameraDevice_init, "<init>", "()V")

        FIND_CLASS (jCameraAccessLevel, "com/pmdtec/jroyale/CameraAccessLevel")
        GET_ENUM_CONSTANT(jCameraAccessLevel, jCameraAccessLevel_L1, "L1", "Lcom/pmdtec/jroyale/CameraAccessLevel;")
        GET_ENUM_CONSTANT(jCameraAccessLevel, jCameraAccessLevel_L2, "L2", "Lcom/pmdtec/jroyale/CameraAccessLevel;")
        GET_ENUM_CONSTANT(jCameraAccessLevel, jCameraAccessLevel_L3, "L3", "Lcom/pmdtec/jroyale/CameraAccessLevel;")
        GET_ENUM_CONSTANT(jCameraAccessLevel, jCameraAccessLevel_L4, "L4", "Lcom/pmdtec/jroyale/CameraAccessLevel;")

        FIND_CLASS (jExposureMode, "com/pmdtec/jroyale/ExposureMode")
        GET_ENUM_CONSTANT(jExposureMode, jExposureMode_MANUAL, "MANUAL", "Lcom/pmdtec/jroyale/ExposureMode;")
        GET_ENUM_CONSTANT(jExposureMode, jExposureMode_AUTOMATIC, "AUTOMATIC", "Lcom/pmdtec/jroyale/ExposureMode;")

        FIND_CLASS (jCamerEvent$Severity, "com/pmdtec/jroyale/data/CameraEvent$Severity")
        GET_ENUM_CONSTANT(jCamerEvent$Severity, jEvent$Severity_INFO, "INFO", "Lcom/pmdtec/jroyale/data/CameraEvent$Severity;")
        GET_ENUM_CONSTANT(jCamerEvent$Severity, jEvent$Severity_WARNING, "WARNING", "Lcom/pmdtec/jroyale/data/CameraEvent$Severity;")
        GET_ENUM_CONSTANT(jCamerEvent$Severity, jEvent$Severity_ERROR, "ERROR", "Lcom/pmdtec/jroyale/data/CameraEvent$Severity;")
        GET_ENUM_CONSTANT(jCamerEvent$Severity, jEvent$Severity_FATAL, "FATAL", "Lcom/pmdtec/jroyale/data/CameraEvent$Severity;")

        FIND_CLASS (jCamerEvent$Type, "com/pmdtec/jroyale/data/CameraEvent$Type")
        GET_ENUM_CONSTANT(jCamerEvent$Type, jEvent$Type_CAPTURE_STREAM, "CAPTURE_STREAM", "Lcom/pmdtec/jroyale/data/CameraEvent$Type;")
        GET_ENUM_CONSTANT(jCamerEvent$Type, jEvent$Type_DEVICE_DISCONNECTED, "DEVICE_DISCONNECTED", "Lcom/pmdtec/jroyale/data/CameraEvent$Type;")
        GET_ENUM_CONSTANT(jCamerEvent$Type, jEvent$Type_OVER_TEMPERATURE, "OVER_TEMPERATURE", "Lcom/pmdtec/jroyale/data/CameraEvent$Type;")
        GET_ENUM_CONSTANT(jCamerEvent$Type, jEvent$Type_RAW_FRAME_STATS, "RAW_FRAME_STATS", "Lcom/pmdtec/jroyale/data/CameraEvent$Type;")

        // initialize handle references

        GET_FIELD (jRoyaleCameraManager, jRoyaleCameraManager_mHandle, "mHandle", "J");

        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mHandle, "mHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mDepthDataListenerHandle, "mDepthDataListenerHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mDepthImageListenerHandle, "mDepthImageListenerHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mEventListenerHandle, "mEventListenerHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mExposureListenerHandle, "mExposureListenerHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mIRImageListenerHandle, "mIRImageListenerHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mRecordStopListenerHandle, "mRecordStopListenerHandle", "J");
        GET_FIELD (jRoyaleCameraDevice, jRoyaleCameraDevice_mSparsePointCloudListenerHandle, "mSparsePointCloudListenerHandle", "J");

        // initialize com.pmdtec.jroyale.data

        FIND_CLASS (jDepthData, "com/pmdtec/jroyale/data/DepthData")
        GET_METHOD (jDepthData, jDepthData_init, "<init>", "(IJJII[J[Lcom/pmdtec/jroyale/data/DepthPoint;)V")

        FIND_CLASS (jDepthPoint, "com/pmdtec/jroyale/data/DepthPoint")
        GET_METHOD (jDepthPoint, jDepthPoint_init, "<init>", "(FFFFII)V")

        FIND_CLASS (jDepthImage, "com/pmdtec/jroyale/data/DepthImage")
        GET_METHOD (jDepthImage, jDepthImage_init, "<init>", "(JIII[I)V")

        FIND_CLASS (jCameraEvent, "com/pmdtec/jroyale/data/CameraEvent")
        GET_STATIC_METHOD (jCameraEvent, jEvent_createEvent, "createEvent", "(Lcom/pmdtec/jroyale/data/CameraEvent$Severity;Ljava/lang/String;Lcom/pmdtec/jroyale/data/CameraEvent$Type;)Lcom/pmdtec/jroyale/data/CameraEvent;")

        FIND_CLASS (jIRImage, "com/pmdtec/jroyale/data/IRImage")
        GET_METHOD (jIRImage, jIRImage_init, "<init>", "(JIII[B)V")

        FIND_CLASS (jLensParameters, "com/pmdtec/jroyale/data/LensParameters")
        GET_METHOD (jLensParameters, jLensParameters_init, "<init>", "(Landroid/util/Pair;Landroid/util/Pair;Landroid/util/Pair;[F)V")

        FIND_CLASS (jSparsePointCloud, "com/pmdtec/jroyale/data/SparsePointCloud")
        GET_METHOD (jSparsePointCloud, jSparsePointCloud_init, "<init>", "(JIJ[F)V")

        // initialize com.pmdtec.jroyale.listener

        FIND_CLASS (jDepthDataListener, "com/pmdtec/jroyale/listener/DepthDataListener")
        GET_METHOD (jDepthDataListener, jDepthDataListener_onNewData, "onNewData", "(Lcom/pmdtec/jroyale/data/DepthData;)V")

        FIND_CLASS (jDepthImageListener, "com/pmdtec/jroyale/listener/DepthImageListener")
        GET_METHOD (jDepthImageListener, jDepthImageListener_onNewData, "onNewData", "(Lcom/pmdtec/jroyale/data/DepthImage;)V")

        FIND_CLASS (jCameraEventListener, "com/pmdtec/jroyale/listener/CameraEventListener")
        GET_METHOD (jCameraEventListener, jCameraEventListener_onEvent, "onEvent", "(Lcom/pmdtec/jroyale/data/CameraEvent;)V")

        FIND_CLASS (jExposureListener, "com/pmdtec/jroyale/listener/ExposureListener")
        GET_METHOD (jExposureListener, jExposureListener_onNewExposure, "onNewExposure", "(JI)V")

        FIND_CLASS (jIRImageListener, "com/pmdtec/jroyale/listener/IRImageListener")
        GET_METHOD (jIRImageListener, jIRImageListener_onNewData, "onNewData", "(Lcom/pmdtec/jroyale/data/IRImage;)V")

        FIND_CLASS (jRecordStopListener, "com/pmdtec/jroyale/listener/RecordStopListener")
        GET_METHOD (jRecordStopListener, jRecordStopListener_onRecordingStopped, "onRecordingStopped", "(J)V")

        FIND_CLASS (jSparsePointCloudListener, "com/pmdtec/jroyale/listener/SparsePointCloudListener")
        GET_METHOD (jSparsePointCloudListener, jSparsePointCloudListener_onNewData, "onNewData", "(Lcom/pmdtec/jroyale/data/SparsePointCloud;)V")

        return JNI_VERSION_1_6;
    }

    JNIEXPORT void JNICALL
    JNI_OnUnload (JavaVM *vm, void *)
    {
        // Obtain the JNIEnv from the VM
        //
        JNIEnv *env;
        vm->GetEnv (reinterpret_cast<void **>(&env), JNI_VERSION_1_6);

        // Destroy the global references
        //
        env->DeleteGlobalRef (jArrayList);
        env->DeleteGlobalRef (jHashMap);

        env->DeleteGlobalRef (jBoolean);
        env->DeleteGlobalRef (jByte);
        env->DeleteGlobalRef (jCharacter);
        env->DeleteGlobalRef (jShort);
        env->DeleteGlobalRef (jInteger);
        env->DeleteGlobalRef (jLong);
        env->DeleteGlobalRef (jFloat);
        env->DeleteGlobalRef (jDouble);

        env->DeleteGlobalRef (jPair);
        env->DeleteGlobalRef (jCameraException);
        env->DeleteGlobalRef (jRoyaleCameraDevice);

        env->DeleteGlobalRef (jCameraAccessLevel);
        env->DeleteGlobalRef (jCameraAccessLevel_L1);
        env->DeleteGlobalRef (jCameraAccessLevel_L2);
        env->DeleteGlobalRef (jCameraAccessLevel_L3);
        env->DeleteGlobalRef (jCameraAccessLevel_L4);

        env->DeleteGlobalRef (jExposureMode);
        env->DeleteGlobalRef (jExposureMode_MANUAL);
        env->DeleteGlobalRef (jExposureMode_AUTOMATIC);

        env->DeleteGlobalRef (jDepthData);
        env->DeleteGlobalRef (jDepthPoint);
        env->DeleteGlobalRef (jDepthImage);
        env->DeleteGlobalRef (jCameraEvent);
        env->DeleteGlobalRef (jIRImage);
        env->DeleteGlobalRef (jLensParameters);
        env->DeleteGlobalRef (jSparsePointCloud);

        env->DeleteGlobalRef (jDepthDataListener);
        env->DeleteGlobalRef (jDepthImageListener);
        env->DeleteGlobalRef (jCameraEventListener);
        env->DeleteGlobalRef (jExposureListener);
        env->DeleteGlobalRef (jIRImageListener);
        env->DeleteGlobalRef (jRecordStopListener);
        env->DeleteGlobalRef (jSparsePointCloudListener);
    }
    }
}
