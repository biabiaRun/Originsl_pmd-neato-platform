/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNICameraDevice.hpp"

namespace jroyale
{
    using namespace royale;

    extern "C" {

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_initialize (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto status = cCameraDevice->initialize ();
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not initialize camera!")
    }

    void finalizeNaive (ICameraDevice *handle, DepthDataListener *depthDataListenerHandle, DepthImageListener *depthImageListenerHandle, EventListener *eventListenerHandle, ExposureListener *exposureListenerHandle, IRImageListener *iRImageListenerHandle, RecordStopListener *recordStopListenerHandle, SparsePointCloudListener *sparsePointCloudListenerHandle)
    {
        if (nullptr != handle)
        { delete (handle); }

        if (nullptr != depthDataListenerHandle)
        { delete (depthDataListenerHandle); }

        if (nullptr != depthImageListenerHandle)
        { delete (depthImageListenerHandle); }

        if (nullptr != eventListenerHandle)
        { delete (eventListenerHandle); }

        if (nullptr != exposureListenerHandle)
        { delete (exposureListenerHandle); }

        if (nullptr != iRImageListenerHandle)
        { delete (iRImageListenerHandle); }

        if (nullptr != recordStopListenerHandle)
        { delete (recordStopListenerHandle); }

        if (nullptr != sparsePointCloudListenerHandle)
        { delete (sparsePointCloudListenerHandle); }
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_finalizeNative (JNIEnv *env, jobject instance)
    {
        auto handle = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto depthDataListenerHandle = handle::get<DepthDataListener> (env, instance, jRoyaleCameraDevice_mDepthDataListenerHandle);
        auto depthImageListenerHandle = handle::get<DepthImageListener> (env, instance, jRoyaleCameraDevice_mDepthImageListenerHandle);
        auto eventListenerHandle = handle::get<EventListener> (env, instance, jRoyaleCameraDevice_mEventListenerHandle);
        auto exposureListenerHandle = handle::get<ExposureListener> (env, instance, jRoyaleCameraDevice_mExposureListenerHandle);
        auto iRImageListenerHandle = handle::get<IRImageListener> (env, instance, jRoyaleCameraDevice_mIRImageListenerHandle);
        auto recordStopListenerHandle = handle::get<RecordStopListener> (env, instance, jRoyaleCameraDevice_mRecordStopListenerHandle);
        auto sparsePointCloudListenerHandle = handle::get<SparsePointCloudListener> (env, instance, jRoyaleCameraDevice_mSparsePointCloudListenerHandle);

        // decouple the deletion of the cpp objects from the java finalize  method, because
        // it has a timeout
        std::thread thread (finalizeNaive, handle, depthDataListenerHandle, depthImageListenerHandle, eventListenerHandle, exposureListenerHandle, iRImageListenerHandle, recordStopListenerHandle, sparsePointCloudListenerHandle);
        thread.detach ();
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_startCapture (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto status = cCameraDevice->startCapture ();
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not start capturing!")
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_stopCapture (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto status = cCameraDevice->stopCapture ();
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not stop capturing!")
    }

    JNIEXPORT jstring JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getId (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        String cID;
        auto status = cCameraDevice->getId (cID);
        THROW_CAMERA_STATUS_IF_NULL("Can not get the camera id!")

        return env->NewStringUTF (cID.c_str ());
    }

    JNIEXPORT jstring JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getCameraName (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        String string;
        auto status = cCameraDevice->getCameraName (string);
        THROW_CAMERA_STATUS_IF_NULL("Can not get the camera name!")

        return env->NewStringUTF (string.c_str ());
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getCameraInfo (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        Vector<Pair<String, String>> cCameraInfo;
        auto status = cCameraDevice->getCameraInfo (cCameraInfo);
        THROW_CAMERA_STATUS_IF_NULL("Can not get the camera info!")

        jobject jCameraInfo = env->NewObject (jHashMap, jHashMap_init, cCameraInfo.size ());

        for (auto it : cCameraInfo)
        {
            auto first = env->NewStringUTF (it.first.c_str ());
            auto second = env->NewStringUTF (it.second.c_str ());

            env->CallObjectMethod (jCameraInfo, jHashMap_put, first, second);
        }

        return jCameraInfo;
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setUseCase (JNIEnv *env, jobject instance, jstring jName)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cName = env->GetStringUTFChars (jName, nullptr);
        auto status = cCameraDevice->setUseCase (cName);
        env->ReleaseStringUTFChars (jName, cName);

        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not set use case")
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getUseCases (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        Vector<String> cUseCases;
        auto status = cCameraDevice->getUseCases (cUseCases);
        THROW_CAMERA_STATUS_IF_NULL("Can not get the camera info!")

        jobject jUseCases = env->NewObject (jArrayList, jArrayList_init, cUseCases.size ());

        for (auto it : cUseCases)
        {
            auto name = env->NewStringUTF (it.c_str ());
            env->CallBooleanMethod (jUseCases, jArrayList_add, name);
        }

        return jUseCases;
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getStreams (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        Vector<StreamId> cStreams;
        auto status = cCameraDevice->getStreams (cStreams);
        THROW_CAMERA_STATUS_IF_NULL("Can not get streams!")

        jobject jStreams = env->NewObject (jArrayList, jArrayList_init, cStreams.size ());

        for (auto cStream : cStreams)
        {
            auto jStream = static_cast<jint> (cStream);
            auto jStreamObj = env->NewObject (jInteger, jInteger_init, jStream);

            env->CallBooleanMethod (jStreams, jArrayList_add, jStreamObj);
            env->DeleteLocalRef (jStreamObj);
        }

        return jStreams;
    }

    JNIEXPORT jlong JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getNumberOfStreams (JNIEnv *env, jobject instance, jstring jUseCase)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        String cUseCase = env->GetStringUTFChars (jUseCase, nullptr);

        uint32_t cNumberOfStreams;
        auto status = cCameraDevice->getNumberOfStreams (cUseCase, cNumberOfStreams);
        THROW_CAMERA_STATUS_IF_NULL_JLONG ("Can not get number of streams!")

        return static_cast<jlong> (cNumberOfStreams);
    }

    JNIEXPORT jstring JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getCurrentUseCase (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        String cUseCase;
        auto status = cCameraDevice->getCurrentUseCase (cUseCase);
        THROW_CAMERA_STATUS_IF_NULL ("Can not get current use case!")

        return env->NewStringUTF (cUseCase.c_str ());
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setExposureTime (JNIEnv *env, jobject instance, jlong jExposureTime, jint jStreamID)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cExposureTime = static_cast<uint32_t> (jExposureTime);
        auto cStreamID = static_cast<uint16_t> (jStreamID);

        auto status = cCameraDevice->setExposureTime (cExposureTime, cStreamID);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not set exposure time!")
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setExposureMode (JNIEnv *env, jobject instance, jobject jExposureMode, jlong jStreamID)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cStreamID = static_cast<uint16_t> (jStreamID);

        ExposureMode cExposureMode = ExposureMode::AUTOMATIC;
        if (env->IsSameObject (jExposureMode, jExposureMode_AUTOMATIC))
        {
            cExposureMode = ExposureMode::AUTOMATIC;
        }
        else if (env->IsSameObject (jExposureMode, jExposureMode_MANUAL))
        {
            cExposureMode = ExposureMode::MANUAL;
        }
        else
        {
            env->ThrowNew (jCameraException, "Unknown exposure mode. Can not be set!");
        }

        auto status = cCameraDevice->getExposureMode (cExposureMode, cStreamID);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not set exposure mode!")
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getExposureMode (JNIEnv *env, jobject instance, jint jStreamID)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cStreamID = static_cast<uint16_t> (jStreamID);

        ExposureMode cExposureMode = ExposureMode::AUTOMATIC;
        auto status = cCameraDevice->getExposureMode (cExposureMode, cStreamID);
        THROW_CAMERA_STATUS_IF_NULL ("Can not get exposure mode!")

        switch (cExposureMode)
        {
            case ExposureMode::AUTOMATIC :
                return jExposureMode_AUTOMATIC;
            case ExposureMode::MANUAL :
                return jExposureMode_MANUAL;
        }
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getExposureLimits (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        Pair<uint32_t, uint32_t> cExposureLimits;
        auto status = cCameraDevice->getExposureLimits (cExposureLimits);
        THROW_CAMERA_STATUS_IF_NULL ("Can not get exposure limits")

        auto jFirst = env->NewObject (jLong, jLong_init, static_cast<jlong> (cExposureLimits.first));
        auto jSecond = env->NewObject (jLong, jLong_init, static_cast<jlong> (cExposureLimits.second));

        auto jExposureLimits = env->NewObject (jPair, jPair_init, jFirst, jSecond);

        env->DeleteLocalRef (jFirst);
        env->DeleteLocalRef (jSecond);

        return jExposureLimits;
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerDataListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new DepthDataListener (env, jListener);
        auto status = cCameraDevice->registerDataListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register DepthDataListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mDepthDataListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterDataListener (JNIEnv *env, jobject instance)
    {
        handle::unset<DepthDataListener> (env, instance, jRoyaleCameraDevice_mDepthDataListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerDepthImageListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new DepthImageListener (env, jListener);
        auto status = cCameraDevice->registerDepthImageListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register DepthImageListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mDepthImageListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterDepthImageListener (JNIEnv *env, jobject instance)
    {
        handle::unset<DepthImageListener> (env, instance, jRoyaleCameraDevice_mDepthImageListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerSparsePointCloudListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new SparsePointCloudListener (env, jListener);
        auto status = cCameraDevice->registerSparsePointCloudListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register SparsePointCloudListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mSparsePointCloudListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterSparsePointCloudListener (JNIEnv *env, jobject instance)
    {
        handle::unset<SparsePointCloudListener> (env, instance, jRoyaleCameraDevice_mSparsePointCloudListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerIRImageListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new IRImageListener (env, jListener);
        auto status = cCameraDevice->registerIRImageListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register IRImageListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mIRImageListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterIRImageListener (JNIEnv *env, jobject instance)
    {
        handle::unset<IRImageListener> (env, instance, jRoyaleCameraDevice_mIRImageListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerEventListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new EventListener (env, jListener);
        auto status = cCameraDevice->registerEventListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register CameraEventListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mEventListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterEventListener (JNIEnv *env, jobject instance)
    {
        handle::unset<EventListener> (env, instance, jRoyaleCameraDevice_mEventListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerRecordListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new RecordStopListener (env, jListener);
        auto status = cCameraDevice->registerRecordListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register RecordListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mRecordStopListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterRecordListener (JNIEnv *env, jobject instance)
    {
        handle::unset<RecordStopListener> (env, instance, jRoyaleCameraDevice_mRecordStopListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_registerExposureListener (JNIEnv *env, jobject instance, jobject jListener)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cListenerPtr = new ExposureListener (env, jListener);
        auto status = cCameraDevice->registerExposureListener (cListenerPtr);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not register ExposureListener!")

        handle::set (env, instance, cListenerPtr, jRoyaleCameraDevice_mExposureListenerHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_unregisterExposureListener (JNIEnv *env, jobject instance)
    {
        handle::unset<ExposureListener> (env, instance, jRoyaleCameraDevice_mExposureListenerHandle);
    }

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getMaxSensorWidth (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        uint16_t cMaxSensorWidth;
        auto status = cCameraDevice->getMaxSensorWidth (cMaxSensorWidth);
        THROW_CAMERA_STATUS_IF_NULL_JINT ("Can not get max sensor height!")

        return static_cast<jint> (cMaxSensorWidth);
    }

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getMaxSensorHeight (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        uint16_t cMaxSensorHeight;
        auto status = cCameraDevice->getMaxSensorHeight (cMaxSensorHeight);
        THROW_CAMERA_STATUS_IF_NULL_JINT ("Can not get max sensor height!")

        return static_cast<jint> (cMaxSensorHeight);
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getLensParameters (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        LensParameters cLensParameters;
        auto status = cCameraDevice->getLensParameters (cLensParameters);
        THROW_CAMERA_STATUS_IF_NULL ("Can not get lens parameters!");

        return createJLensParameters (env, cLensParameters);
    }

    JNIEXPORT jboolean JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_isConnected (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        bool cConnected;
        auto status = cCameraDevice->isConnected (cConnected);
        THROW_CAMERA_STATUS_IF_NULL_JBOOL ("Can not request if connected!")

        return static_cast<jboolean> (cConnected);
    }

    JNIEXPORT jboolean JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_isCalibrated (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        bool cCalibrated;
        auto status = cCameraDevice->isCalibrated (cCalibrated);
        THROW_CAMERA_STATUS_IF_NULL_JBOOL ("Can not request if calibrated!")

        return static_cast<jboolean> (cCalibrated);
    }

    JNIEXPORT jboolean JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_isCapturing (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        bool cCapturing;
        auto status = cCameraDevice->isCapturing (cCapturing);
        THROW_CAMERA_STATUS_IF_NULL_JBOOL ("Can not request if capturing!")

        return static_cast<jboolean> (cCapturing);
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getAccessLevel (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        CameraAccessLevel cCameraAccessLevel;
        auto status = cCameraDevice->getAccessLevel (cCameraAccessLevel);
        THROW_CAMERA_STATUS_IF_NULL ("Can not get the access level");

        switch (cCameraAccessLevel)
        {
            case CameraAccessLevel::L1 :
                return jCameraAccessLevel_L1;
            case CameraAccessLevel::L2 :
                return jCameraAccessLevel_L2;
            case CameraAccessLevel::L3 :
                return jCameraAccessLevel_L3;
            case CameraAccessLevel::L4 :
                return jCameraAccessLevel_L4;
        }
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_startRecording (JNIEnv *env, jobject instance, jstring jFileName, jlong jNumberOfFrames, jlong jFrameSkip, jlong jMSSkip)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        String cFileName = env->GetStringUTFChars (jFileName, nullptr);
        auto cNumberOfFrames = static_cast<uint32_t> (jNumberOfFrames);
        auto cFrameSkip = static_cast<uint32_t> (jFrameSkip);
        auto cMSSkip = static_cast<uint32_t> (jMSSkip);

        auto status = cCameraDevice->startRecording (cFileName, cNumberOfFrames, cFrameSkip, cMSSkip);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not start recording!");
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_stopRecording (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto status = cCameraDevice->stopRecording ();
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not stop recording!");
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setFrameRate (JNIEnv *env, jobject instance, jint jFrameRate)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cFrameRate = static_cast<uint16_t> (jFrameRate);
        auto status = cCameraDevice->setFrameRate (cFrameRate);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not set frame rate")
    }

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getFrameRate (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        uint16_t cFrameRate;
        auto status = cCameraDevice->getFrameRate (cFrameRate);
        THROW_CAMERA_STATUS_IF_NULL_JINT ("Can not get max frame rate!");

        return static_cast<jint> (cFrameRate);
    }

    JNIEXPORT jint JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_getMaxFrameRate (JNIEnv *env, jobject instance)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        uint16_t cMaxFrameRate;
        auto status = cCameraDevice->getMaxFrameRate (cMaxFrameRate);
        THROW_CAMERA_STATUS_IF_NULL_JINT ("Can not get max frame rate!");

        return static_cast<jint> (cMaxFrameRate);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraDevice_setExternalTrigger (JNIEnv *env, jobject instance, jboolean jExternalTrigger)
    {
        auto cCameraDevice = handle::get<ICameraDevice> (env, instance, jRoyaleCameraDevice_mHandle);

        auto cExternalTrigger = static_cast<bool> (jExternalTrigger);
        auto status = cCameraDevice->setExternalTrigger (cExternalTrigger);
        THROW_CAMERA_STATUS_IF_NULL_VOID ("Can not set external trigger")
    }
    }
}
