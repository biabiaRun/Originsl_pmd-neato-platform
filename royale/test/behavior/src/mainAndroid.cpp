#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Helper.hpp"
#include <iostream>
#include <jni.h>
#include <string>
#include <android/log.h>

int ANDROID_USB_DEVICE_FD;

#ifdef __cplusplus
extern "C"
{
#endif

jint Java_com_royale_royalebdd_MainActivity_runBDD (JNIEnv *env, jobject thiz, jint androidUsbFD)
{
    LOG_OUTPUT ("USB FD: %d", androidUsbFD);
    ANDROID_USB_DEVICE_FD = androidUsbFD;

    int nargc = 3;
    const char **nargv = (const char **) malloc (sizeof (char *) * 3);

    nargv[0] = "text_royale_bdd";
    nargv[1] = "-o";
    nargv[2] = (ANDROID_OUTPUT_DIRECTORY + "royale_bdd_report.txt").c_str();

    int ret_val = Catch::Session().run (nargc, nargv);

    free (nargv);

    return ret_val;
}

#ifdef __cplusplus
}
#endif