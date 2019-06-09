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
#include <royale.hpp>

#include "JavaVM.hpp"

namespace jroyale
{
    using namespace royale;

    jfloatArray createJFloatArray (JNIEnv *env, Vector<float> cFloatVector);

    jbyteArray createJUByteArray (JNIEnv *env, Vector<uint8_t> cUInt8Vector);

    jbyteArray createJByteArray (JNIEnv *env, Vector<int8_t> cInt8Vector);

    jcharArray createJUCharArray (JNIEnv *env, Vector<uint8_t> cUInt8Vector);

    jcharArray createJCharArray (JNIEnv *env, Vector<int16_t> cInt16Vector);

    jintArray createJUIntArray (JNIEnv *env, Vector<uint16_t> cUInt16Vector);

    jintArray createJIntArray (JNIEnv *env, Vector<int32_t> cInt32Vector);

    jlongArray createJULongArray (JNIEnv *env, Vector<uint32_t> cUInt32Vector);

    jlongArray createJLongArray (JNIEnv *env, Vector<int64_t> cInt64Vector);
}
