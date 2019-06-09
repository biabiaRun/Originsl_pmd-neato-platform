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
#include <mutex>

#include <royale/IEventListener.hpp>
#include <royale/IEvent.hpp>

#include "JNIEvent.hpp"
#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    class EventListener : public IEventListener
    {
    public:

        EventListener (JNIEnv *, jobject);

        ~EventListener ();

        void onEvent (std::unique_ptr<royale::IEvent> &&) override;

    private:

        jobject jListener;
        std::mutex jListenerMutex;
    };
}
