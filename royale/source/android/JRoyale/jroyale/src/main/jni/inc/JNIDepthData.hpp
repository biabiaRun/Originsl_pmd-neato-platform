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
#include <chrono>
#include <cstdint>

#include <royale/DepthData.hpp>
#include <royale/Vector.hpp>

#include "JavaVM.hpp"
#include "JNIUtils.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJDepthData (JNIEnv *, const DepthData &);
}
