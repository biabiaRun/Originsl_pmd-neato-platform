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

#include <royale/ISparsePointCloudListener.hpp>
#include <royale/SparsePointCloud.hpp>

#include "JNISparsePointCloud.hpp"
#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    class SparsePointCloudListener : public ISparsePointCloudListener
    {
    public:

        SparsePointCloudListener (JNIEnv *, jobject);

        ~SparsePointCloudListener ();

        void onNewData (const royale::SparsePointCloud *) override;

    private:

        jobject jListener;

        std::mutex jListenerMutex;
    };
}
