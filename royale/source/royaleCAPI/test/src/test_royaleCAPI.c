/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <unity.h>
#include <CameraManagerCAPI.h>
#include <CameraDeviceCAPI.h>
#include <DepthDataCAPI.h>
#include <PlatformResourcesCAPI.h>
#include <TestCameraDeviceL1.h>
#include <TestCameraDeviceL2.h>
#include <TestCameraDeviceL3.h>
#include <TestCameraDeviceL4.h>
#include <TestHelpers.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void setUp (void)
{
}

void tearDown (void)
{
}

int main (void)
{
    UNITY_BEGIN();

    royale_camera_status status = royale_platform_resources_initialize();
    if (ROYALE_STATUS_SUCCESS != status)
    {
        printf ("Failed to initialize platform resources: %d", status);
        return -1;
    }

    unsigned major, minor, patch, build;
    char *scm_rev;
    royale_get_version_with_build_and_scm_revision (&major, &minor, &patch, &build, &scm_rev);
    printf ("===========================================================\n");
    printf ("testing royaleCAPI %d for version %u.%u.%u.%u (%s)\n", ROYALE_C_API_VERSION, major, minor, patch, build, scm_rev);
    printf ("===========================================================\n");
    royale_free_string (scm_rev);

    printf ("===========================================================\n");
    printf ("testing royaleCAPI LEVEL 1\n");
    printf ("===========================================================\n");

    test_royaleCAPI_CameraDevice_Level1();

    printf ("===========================================================\n");
    printf ("testing royaleCAPI LEVEL 2\n");
    printf ("===========================================================\n");

    test_royaleCAPI_CameraDevice_Level2();

    printf ("===========================================================\n");
    printf ("testing royaleCAPI LEVEL 3\n");
    printf ("===========================================================\n");

    test_royaleCAPI_CameraDevice_Level3();

    printf ("===========================================================\n");
    printf ("testing royaleCAPI LEVEL 4\n");
    printf ("===========================================================\n");

    test_royaleCAPI_CameraDevice_Level4();

    printf ("===========================================================\n");
    printf ("finished testing royaleCAPI %d for version %u.%u.%u.%u\n", ROYALE_C_API_VERSION, major, minor, patch, build);
    printf ("===========================================================\n");

    royale_platform_resources_uninitialize();

    return UNITY_END();
}
