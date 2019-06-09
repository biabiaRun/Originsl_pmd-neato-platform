/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIIRImageListener.hpp"

namespace jroyale
{
    using namespace royale;

    IRImageListener::IRImageListener (JNIEnv *env, jobject jListener)
    {
        this->jListener = env->NewGlobalRef (jListener);
    }

    IRImageListener::~IRImageListener ()
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        std::lock_guard<std::mutex> lock (jListenerMutex);
        env->DeleteGlobalRef (this->jListener);
        this->jListener = nullptr;
    }

    void IRImageListener::onNewData (const IRImage *data)
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        auto tmpData = createJIRImage (env, *data);
        {
            std::lock_guard<std::mutex> lock (jListenerMutex);
            if (this->jListener != nullptr)
            {
                env->CallVoidMethod (this->jListener, jIRImageListener_onNewData, tmpData);
            }
        }
        env->DeleteLocalRef (tmpData);
    };
}
