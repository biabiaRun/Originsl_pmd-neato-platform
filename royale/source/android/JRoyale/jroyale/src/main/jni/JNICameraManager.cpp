/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNICameraManager.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createCamera (JNIEnv *env, jobject instance, CameraManager *cCameraManager, const String cStreamId)
    {
        // instantiate tha camera device with error check
        auto cCameraDevice = cCameraManager->createCamera (cStreamId);
        if (NULL == cCameraDevice)
        {
            env->ThrowNew (jCameraException, "Camera can not be created!");
            return NULL;
        }

        auto jCameraDeviceObj = env->NewObject (jRoyaleCameraDevice, jRoyaleCameraDevice_init);

        auto cCameraHandle = cCameraDevice.get ();
        handle::set (env, jCameraDeviceObj, cCameraHandle, jRoyaleCameraDevice_mHandle);

        // the camera device is now managed by the lifetime
        // of if java camera device object
        cCameraDevice.release ();

        return jCameraDeviceObj;
    }

    extern "C" {

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_initializeNative (JNIEnv *env, jobject instance, jstring jActivationCode)
    {
        // instantiate the camera manager
        auto cActivationCode = env->GetStringUTFChars (jActivationCode, nullptr);
        auto cCameraManager = new CameraManager (cActivationCode);
        env->ReleaseStringUTFChars (jActivationCode, cActivationCode);

        // store the camera manager pointer in the instance
        handle::set (env, instance, cCameraManager, jRoyaleCameraManager_mHandle);
    }

    JNIEXPORT void JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_finalizeNative (JNIEnv *env, jobject instance)
    {
        // receive the camera manager from the instance and release it
        handle::unset<CameraManager> (env, instance, jRoyaleCameraManager_mHandle);
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_getAccessLevel (JNIEnv *env, jobject, jstring jActivationCode)
    {
        auto cActivationCode = env->GetStringUTFChars (jActivationCode, nullptr);
        auto cAccessLevel = CameraManager::getAccessLevel (cActivationCode);
        env->ReleaseStringUTFChars (jActivationCode, cActivationCode);

        switch (cAccessLevel)
        {
            case CameraAccessLevel::L1:
                return jCameraAccessLevel_L1;

            case CameraAccessLevel::L2:
                return jCameraAccessLevel_L2;

            case CameraAccessLevel::L3:
                return jCameraAccessLevel_L3;

            case CameraAccessLevel::L4:
                return jCameraAccessLevel_L4;
        }
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_createCamera__JJJ (JNIEnv *env, jobject instance, jlong jFileDescriptor, jlong jVersionId, jlong jProductId)
    {
        auto cCameraManager = handle::get<CameraManager> (env, instance, jRoyaleCameraManager_mHandle);

        auto cFileDescriptor = static_cast<uint32_t> (jFileDescriptor);
        auto cVersionId = static_cast<uint32_t> (jVersionId);
        auto cProductId = static_cast<uint32_t> (jProductId);

        auto streamIds = cCameraManager->getConnectedCameraList (cFileDescriptor, cVersionId, cProductId);
        if (1 > streamIds.count ())
        {
            env->ThrowNew (jCameraException, "No Camera was found!");
            return NULL;
        }

        auto cStreamId = streamIds.at (0);
        return createCamera (env, instance, cCameraManager, cStreamId);
    }

    JNIEXPORT jobject JNICALL
    Java_com_pmdtec_jroyale_RoyaleCameraManager_createCamera__Ljava_lang_String_2 (JNIEnv *env, jobject instance, jstring jStreamId)
    {
        auto cCameraManager = handle::get<CameraManager> (env, instance, jRoyaleCameraManager_mHandle);

        auto cStreamId = env->GetStringUTFChars (jStreamId, nullptr);
        auto jCameraDevice = createCamera (env, instance, cCameraManager, cStreamId);
        env->ReleaseStringUTFChars (jStreamId, cStreamId);

        return jCameraDevice;
    }
    }
}
