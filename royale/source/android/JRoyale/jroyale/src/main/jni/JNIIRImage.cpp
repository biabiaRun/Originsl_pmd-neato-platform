/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIIRImage.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJIRImage (JNIEnv *env, const IRImage &irImage)
    {
        auto jTimeStamp = static_cast<jlong> ( irImage.timestamp );
        auto jStreamId = static_cast<jint> ( irImage.streamId );
        auto jWidth = static_cast<jint> ( irImage.width );
        auto jHeight = static_cast<jint> ( irImage.height );
        auto jData = createJUByteArray (env, irImage.data);

        return env->NewObject (jIRImage, jIRImage_init, jTimeStamp, jStreamId, jWidth, jHeight, jData);
    }
}
