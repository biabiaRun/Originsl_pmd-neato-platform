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

#include <royale/IExposureListener2.hpp>

#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    class ExposureListener : public IExposureListener2
    {
    public:

        ExposureListener (JNIEnv *, jobject);

        ~ExposureListener ();

        void onNewExposure (const uint32_t, const royale::StreamId) override;

    private:

        jobject jListener;
        std::mutex jListenerMutex;
    };
}
