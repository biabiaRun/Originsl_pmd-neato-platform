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

namespace jroyale
{
    namespace handle
    {
        template<typename T>
        T *get (JNIEnv *env, jobject instance, jfieldID handleField)
        {
            auto handleValue = env->GetLongField (instance, handleField);

            return reinterpret_cast<T *>(handleValue);
        }

        template<typename T>
        T *exchange (JNIEnv *env, jobject instance, T *newHandle, jfieldID handleField)
        {
            auto newHandleValue = reinterpret_cast<jlong>(newHandle);

            auto oldHandleValue = env->GetLongField (instance, handleField);
            auto oldHandle = reinterpret_cast<T *>(oldHandleValue);

            env->SetLongField (instance, handleField, newHandleValue);
            return oldHandle;
        }

        template<typename T>
        void set (JNIEnv *env, jobject instance, T *newHandle, jfieldID handleField)
        {
            auto oldHandle = exchange<T> (env, instance, newHandle, handleField);

            if (oldHandle != nullptr)
            {
                env->SetLongField (instance, handleField, static_cast<jlong> (0));
                delete (oldHandle);
            }
        }

        template<typename T>
        void unset (JNIEnv *env, jobject instance, jfieldID handleField)
        {
            auto oldHandleValue = env->GetLongField (instance, handleField);
            auto oldHandle = reinterpret_cast<T *>(oldHandleValue);

            if (oldHandle != nullptr)
            {
                env->SetLongField (instance, handleField, static_cast<jlong> (0));
                delete (oldHandle);
            }
        }
    }
}
