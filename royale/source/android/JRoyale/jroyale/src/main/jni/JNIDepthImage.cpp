/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIDepthImage.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJDepthImage (JNIEnv *env, const DepthImage &depthImage)
    {
        auto jTimeStamp = static_cast<jlong> (depthImage.timestamp);
        auto jStreamID = static_cast<jint> (depthImage.streamId);
        auto jWidth = static_cast<jint> (depthImage.width);
        auto jHeight = static_cast<jint> (depthImage.height);
        auto jData = createJUIntArray (env, depthImage.data);

        return env->NewObject (jDepthImage, jDepthImage_init, jTimeStamp, jStreamID, jWidth, jHeight, jData);
    }
}
