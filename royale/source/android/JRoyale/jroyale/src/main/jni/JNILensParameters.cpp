/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNILensParameters.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJFloatPair (JNIEnv *env, Pair<float, float> cPair)
    {
        auto jFirst = static_cast<jfloat> (cPair.first);
        auto jSecond = static_cast<jfloat> (cPair.second);

        auto jFirstObj = env->NewObject (jFloat, jFloat_init, jFirst);
        auto jSecondObj = env->NewObject (jFloat, jFloat_init, jSecond);

        auto jFloatPair = env->NewObject (jPair, jPair_init, jFirstObj, jSecondObj);
        env->DeleteLocalRef (jFirstObj);
        env->DeleteLocalRef (jSecondObj);

        return jFloatPair;
    }

    jobject createJLensParameters (JNIEnv *env, const LensParameters &lensParameters)
    {
        auto jPrincipalPoint = createJFloatPair (env, lensParameters.principalPoint);
        auto jFocalLength = createJFloatPair (env, lensParameters.focalLength);
        auto jDistortionTangential = createJFloatPair (env, lensParameters.distortionTangential);
        auto jDistortionRadial = createJFloatArray (env, lensParameters.distortionRadial);

        return env->NewObject (jLensParameters, jLensParameters_init, jPrincipalPoint, jFocalLength, jDistortionTangential, jDistortionRadial);
    }
}
