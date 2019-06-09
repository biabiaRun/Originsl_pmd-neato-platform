/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIRecordStopListener.hpp"

namespace jroyale
{
    using namespace royale;

    RecordStopListener::RecordStopListener (JNIEnv *env, jobject jListener)
    {
        this->jListener = env->NewGlobalRef (jListener);
    }

    RecordStopListener::~RecordStopListener ()
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        std::lock_guard<std::mutex> lock (jListenerMutex);
        env->DeleteGlobalRef (this->jListener);
        this->jListener = nullptr;
    }

    void RecordStopListener::onRecordingStopped (const uint32_t numFrames)
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        auto tmpData = static_cast<jlong >(numFrames);
        {
            std::lock_guard<std::mutex> lock (jListenerMutex);
            if (this->jListener != nullptr)
            {
                env->CallVoidMethod (this->jListener, jRecordStopListener_onRecordingStopped, tmpData);
            }
        }
    };
}
