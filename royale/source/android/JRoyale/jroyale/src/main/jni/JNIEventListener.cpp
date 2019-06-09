/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIEventListener.hpp"

namespace jroyale
{
    using namespace royale;

    EventListener::EventListener (JNIEnv *env, jobject jListener)
    {
        this->jListener = env->NewGlobalRef (jListener);
    }

    EventListener::~EventListener ()
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        std::lock_guard<std::mutex> lock (jListenerMutex);
        env->DeleteGlobalRef (this->jListener);
        this->jListener = nullptr;
    }

    void EventListener::onEvent (std::unique_ptr<royale::IEvent> &&event)
    {
        JNIEnv *env;
        javaVM->AttachCurrentThread (&env, NULL);

        auto jEventObj = createJEvent (env, event);
        {
            std::lock_guard<std::mutex> lock (jListenerMutex);
            if (this->jListener != nullptr)
            {
                env->CallVoidMethod (this->jListener, jCameraEventListener_onEvent, jEventObj);
            }
        }
        env->DeleteLocalRef (jEventObj);
    };
}
