/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNISparsePointCloud.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJSparsePointCloud (JNIEnv *env, const SparsePointCloud &sparsePointCloud)
    {
        auto jStreamId = static_cast<jint> (sparsePointCloud.streamId);
        auto jTimestamp = static_cast<jlong> (sparsePointCloud.timestamp);
        auto jNumPoints = static_cast<jlong> (sparsePointCloud.numPoints);
        auto jXYZCPoints = createJFloatArray (env, sparsePointCloud.xyzcPoints);

        return env->NewObject (jSparsePointCloud, jSparsePointCloud_init, jStreamId, jTimestamp, jNumPoints, jXYZCPoints);
    }

}
