/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIUtils.hpp"

namespace jroyale
{
    using namespace royale;

    jfloatArray createJFloatArray (JNIEnv *env, Vector<float> cFloatVector)
    {
        auto cCount = cFloatVector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jfloat jFloatArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jFloatArrayBuffer[i] = static_cast<jfloat> (cFloatVector[i]);
        }

        auto jFloatArray = env->NewFloatArray (jCount);
        env->SetFloatArrayRegion (jFloatArray, 0, jCount, jFloatArrayBuffer);

        return jFloatArray;
    }

    jbyteArray createJUByteArray (JNIEnv *env, Vector<uint8_t> cUInt8Vector)
    {
        auto cCount = cUInt8Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jbyte jByteArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jByteArrayBuffer[i] = static_cast<jbyte> (cUInt8Vector[i]);
        }

        auto jByteArray = env->NewByteArray (jCount);
        env->SetByteArrayRegion (jByteArray, 0, jCount, jByteArrayBuffer);

        return jByteArray;
    }

    jbyteArray createJByteArray (JNIEnv *env, Vector<int8_t> cInt8Vector)
    {
        auto cCount = cInt8Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jbyte jByteArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jByteArrayBuffer[i] = static_cast<jbyte> (cInt8Vector[i]);
        }

        auto jByteArray = env->NewByteArray (jCount);
        env->SetByteArrayRegion (jByteArray, 0, jCount, jByteArrayBuffer);

        return jByteArray;
    }

    jcharArray createJUCharArray (JNIEnv *env, Vector<uint8_t> cUInt8Vector)
    {
        auto cCount = cUInt8Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jchar jCharArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jCharArrayBuffer[i] = static_cast<jchar> (cUInt8Vector[i]);
        }

        auto jCharArray = env->NewCharArray (jCount);
        env->SetCharArrayRegion (jCharArray, 0, jCount, jCharArrayBuffer);

        return jCharArray;
    }

    jcharArray createJCharArray (JNIEnv *env, Vector<int16_t> cInt16Vector)
    {
        auto cCount = cInt16Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jchar jCharArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jCharArrayBuffer[i] = static_cast<jchar> (cInt16Vector[i]);
        }

        auto jCharArray = env->NewCharArray (jCount);
        env->SetCharArrayRegion (jCharArray, 0, jCount, jCharArrayBuffer);

        return jCharArray;
    }

    jintArray createJUIntArray (JNIEnv *env, Vector<uint16_t> cUInt16Vector)
    {
        auto cCount = cUInt16Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jint jIntArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jIntArrayBuffer[i] = static_cast<jint> (cUInt16Vector[i]);
        }

        auto jIntArray = env->NewIntArray (jCount);
        env->SetIntArrayRegion (jIntArray, 0, jCount, jIntArrayBuffer);

        return jIntArray;
    }

    jintArray createJIntArray (JNIEnv *env, Vector<int32_t> cInt32Vector)
    {
        auto cCount = cInt32Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jint jIntArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jIntArrayBuffer[i] = static_cast<jint> (cInt32Vector[i]);
        }

        auto jIntArray = env->NewIntArray (jCount);
        env->SetIntArrayRegion (jIntArray, 0, jCount, jIntArrayBuffer);

        return jIntArray;
    }

    jlongArray createJULongArray (JNIEnv *env, Vector<uint32_t> cUInt32Vector)
    {
        auto cCount = cUInt32Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jlong jLongArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jLongArrayBuffer[i] = static_cast<jlong> (cUInt32Vector[i]);
        }

        auto jLongArray = env->NewLongArray (jCount);
        env->SetLongArrayRegion (jLongArray, 0, jCount, jLongArrayBuffer);

        return jLongArray;
    }

    jlongArray createJLongArray (JNIEnv *env, Vector<int64_t> cInt64Vector)
    {
        auto cCount = cInt64Vector.count ();
        auto jCount = static_cast<jsize> (cCount);

        jlong jLongArrayBuffer[cCount];
        for (size_t i = 0; i < cCount; i++)
        {
            jLongArrayBuffer[i] = static_cast<jlong> (cInt64Vector[i]);
        }

        auto jLongArray = env->NewLongArray (jCount);
        env->SetLongArrayRegion (jLongArray, 0, jCount, jLongArrayBuffer);

        return jLongArray;
    }
}
