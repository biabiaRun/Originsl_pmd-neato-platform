/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIExposureListener.hpp"

namespace jroyale
{
    using namespace royale;

    ExposureListener::ExposureListener (JNIEnv *env, jobject jListener)
    {
        this->jListener = env->NewGlobalRef (jListener);
    }

    ExposureListener::~ExposureListener ()
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        std::lock_guard<std::mutex> lock (jListenerMutex);
        env->DeleteGlobalRef (this->jListener);
        this->jListener = nullptr;
    }


    void ExposureListener::onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId)
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        auto tmpData = static_cast<jlong> (exposureTime);
        auto tmpStream = static_cast<jint> (streamId);
        {
            std::lock_guard<std::mutex> lock (jListenerMutex);
            if (this->jListener != nullptr)
            {
                env->CallVoidMethod (this->jListener, jExposureListener_onNewExposure, tmpData, tmpStream);
            }
        }
    }
}
