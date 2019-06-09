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

#include <royale/IIRImageListener.hpp>
#include <royale/IRImage.hpp>

#include "JNIIRImage.hpp"
#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    class IRImageListener : public IIRImageListener
    {
    public:

        IRImageListener (JNIEnv *, jobject);

        ~IRImageListener ();

        void onNewData (const royale::IRImage *) override;

    private:

        jobject jListener;
        std::mutex jListenerMutex;
    };
}
