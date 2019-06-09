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

#include <royale/IDepthImageListener.hpp>
#include <royale/DepthImage.hpp>

#include "JNIDepthImage.hpp"
#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    class DepthImageListener : public IDepthImageListener
    {
    public:

        DepthImageListener (JNIEnv *, jobject);

        ~DepthImageListener ();

        void onNewData (const DepthImage *) override;

    private:

        jobject jListener;
        std::mutex jListenerMutex;
    };
}
