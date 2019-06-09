/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIDepthData.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJDepthPoint (JNIEnv *env, const DepthPoint &cDepthPoint)
    {
        auto jX = static_cast<jfloat> (cDepthPoint.x);
        auto jY = static_cast<jfloat> (cDepthPoint.y);
        auto jZ = static_cast<jfloat> (cDepthPoint.z);
        auto jNoise = static_cast<jfloat> (cDepthPoint.noise);
        auto jGrayValue = static_cast<jint> (cDepthPoint.grayValue);
        auto jDepthConfidence = static_cast<jint> (cDepthPoint.depthConfidence);

        auto jDepthPointObj = env->NewObject (jDepthPoint, jDepthPoint_init, jX, jY, jZ, jNoise, jGrayValue, jDepthConfidence);

        return jDepthPointObj;
    }

    jobjectArray createJDepthPointArray (JNIEnv *env, const Vector<DepthPoint> &cDepthPointVector)
    {
        auto cCount = cDepthPointVector.count ();
        auto jCount = static_cast<jsize> (cCount);

        auto jDepthPointArray = env->NewObjectArray (jCount, jDepthPoint, NULL);

        for (size_t cIndex = 0; cIndex < cCount; cIndex++)
        {
            auto jIndex = static_cast<jsize> (cIndex);

            auto jDepthPointObj = createJDepthPoint (env, cDepthPointVector[cIndex]);
            env->SetObjectArrayElement (jDepthPointArray, jIndex, jDepthPointObj);
            env->DeleteLocalRef (jDepthPointObj);
        }

        return jDepthPointArray;
    }

    jobject createJDepthData (JNIEnv *env, const DepthData &depthData)
    {
        auto jVersion = static_cast<jint> (depthData.version);
        auto jTimestamp = static_cast<jlong> (depthData.timeStamp.count ());
        auto jStreamId = static_cast<jlong> (depthData.streamId);
        auto jWidth = static_cast<jint> (depthData.width);
        auto jHeight = static_cast<jint> (depthData.height);
        auto jExposureTimeArray = createJULongArray (env, depthData.exposureTimes);
        auto jDepthPointArray = createJDepthPointArray (env, depthData.points);

        auto jDepthDataObj = env->NewObject (jDepthData, jDepthData_init, jVersion, jTimestamp, jStreamId, jWidth, jHeight, jExposureTimeArray, jDepthPointArray);

        env->DeleteLocalRef (jExposureTimeArray);
        env->DeleteLocalRef (jDepthPointArray);

        return jDepthDataObj;
    }
}
