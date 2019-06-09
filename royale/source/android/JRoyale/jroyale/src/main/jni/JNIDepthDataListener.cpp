/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIDepthDataListener.hpp"

namespace jroyale
{
    using namespace royale;

    DepthDataListener::DepthDataListener (JNIEnv *env, jobject jListener)
    {
        this->jListener = env->NewGlobalRef (jListener);
    }

    DepthDataListener::~DepthDataListener ()
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        std::lock_guard<std::mutex> lock (jListenerMutex);
        env->DeleteGlobalRef (this->jListener);
        this->jListener = nullptr;
    }

    void DepthDataListener::onNewData (const DepthData *data)
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        auto tmpJDepthData = createJDepthData (env, *data);
        {
            std::lock_guard<std::mutex> lock (this->jListenerMutex);
            if (this->jListener != nullptr)
            {
                env->CallVoidMethod (this->jListener, jDepthDataListener_onNewData, tmpJDepthData);
            }
        }
        env->DeleteLocalRef (tmpJDepthData);
    };
}
