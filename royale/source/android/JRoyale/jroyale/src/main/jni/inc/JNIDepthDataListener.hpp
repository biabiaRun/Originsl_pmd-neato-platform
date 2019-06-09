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

#include <royale/IDepthDataListener.hpp>
#include <royale/DepthData.hpp>

#include "JNIDepthData.hpp"
#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    class DepthDataListener : public IDepthDataListener
    {
    public:

        DepthDataListener (JNIEnv *, jobject);

        ~DepthDataListener ();

        void onNewData (const DepthData *) override;

    private:

        jobject jListener;
        std::mutex jListenerMutex;
    };
}
