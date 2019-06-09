#pragma once

#include "catch.hpp"
#include <royale.hpp>

using namespace royale;
using namespace std;

#if defined(TARGET_PLATFORM_ANDROID)
extern int ANDROID_USB_DEVICE_FD;

#include <android/log.h>

#define  LOG_TAG    "ROYALE_BDD"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOG_OUTPUT(...) LOGD(__VA_ARGS__)
#define ANDROID_OUTPUT_DIRECTORY std::string("/storage/emulated/0/bdd/")
#else
#define LOG_OUTPUT(...) printf(__VA_ARGS__)
#define ANDROID_OUTPUT_DIRECTORY std::string("")
#endif

void CHECK_ROYALE_SUCCESS (CameraStatus status);
