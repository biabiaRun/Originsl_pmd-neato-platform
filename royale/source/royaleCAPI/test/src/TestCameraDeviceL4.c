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
#include <string.h>

#include <royaleCAPI.h>
#include <CameraManagerCAPI.h>
#include <TestHelpers.h>
#include <TestCameraDeviceL4.h>

royale_camera_handle cam_hnd;
royale_camera_status status;

void test_level4_create_manager_and_init_camera (void)
{
    TEST_PRINT_FUNCTION_NAME;

    royale_cam_manager_hnd man_hnd = royale_camera_manager_create_with_code (ROYALE_ACCESS_CODE_LEVEL4);
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, man_hnd, "Failed to create camera manager LEVEL 4.");

    // get connected cameras
    uint32_t nr_cameras = UINT32_MAX;
    char **camera_list = royale_camera_manager_get_connected_cameras (man_hnd, &nr_cameras);
    TEST_ASSERT_NOT_EQUAL_MESSAGE (UINT32_MAX, nr_cameras, "Failed to get connected cameras.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, nr_cameras, "No connected cameras found.");

    // open first camera in list
    cam_hnd = royale_camera_manager_create_camera (man_hnd, camera_list[0]);
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, cam_hnd, "Failed to create camera.");

    //try to open the same camera another time -> should return NULL
    royale_camera_handle invalid_cam_hnd = royale_camera_manager_create_camera (man_hnd, camera_list[0]);
    TEST_ASSERT_EQUAL_MESSAGE (0, invalid_cam_hnd, "Same camera opened multiple times -> not allowed.");
    royale_free_string_array (camera_list, nr_cameras);

    // initialize camera with use case
    char *cam_name;
    status = royale_camera_device_get_camera_name (cam_hnd, &cam_name);

    const char *use_case;
    if (strcmp (cam_name, "PICOFLEXX") == 0)
    {
        const char *use_case_picoflexx = "MODE_9_5FPS_2000";
        use_case = use_case_picoflexx;
    }
    else if (strcmp (cam_name, "PICOMAXX1") == 0 ||
             strcmp (cam_name, "PICOMAXX2") == 0 ||
             strcmp (cam_name, "PICOMONSTAR1") == 0 ||
             strcmp (cam_name, "PICOMONSTAR2") == 0)
    {
        const char *use_case_picomaxx = "MODE_9_5FPS_1900";
        use_case = use_case_picomaxx;
    }
    else
    {
        // not yet supported for this test
        royale_free_string (cam_name);
        return;
    }

    royale_free_string (cam_name);

    status = royale_camera_device_initialize_with_use_case (cam_hnd, use_case);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to initialize camera.");

    char *current_uc = NULL;
    status = royale_camera_device_get_current_use_case (cam_hnd, &current_uc);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get current use case.");
    TEST_ASSERT_EQUAL_MESSAGE (0, strcmp (use_case, current_uc), "Camera was initialized with wrong use case.");
    royale_free_string (current_uc);

    // check access level
    royale_camera_access_level level;
    status = royale_camera_device_get_access_level (cam_hnd, &level);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get access level.");
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_ACCESS_LEVEL4, level, "Camera has wrong access level.");

    // check flags
    bool is_connected;
    status = royale_camera_device_is_connected (cam_hnd, &is_connected);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to check if camera is connected.");
    TEST_ASSERT_TRUE_MESSAGE (is_connected, "Camera connected flag is invalid.");

    bool is_calibrated;
    status = royale_camera_device_is_calibrated (cam_hnd, &is_calibrated);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to check if camera is calibrated.");
    TEST_ASSERT_TRUE_MESSAGE (is_calibrated, "Camera calibrated flag is invalid");

    bool is_capturing;
    status = royale_camera_device_is_capturing (cam_hnd, &is_capturing);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to check if camera is capturing.");
    TEST_ASSERT_FALSE_MESSAGE (is_capturing, "Camera capturing flag is true before start capture call.");

    // start capture
    status = royale_camera_device_start_capture (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to start capturing.");

    status = royale_camera_device_is_capturing (cam_hnd, &is_capturing);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to check if camera is capturing.");
    TEST_ASSERT_TRUE_MESSAGE (is_capturing, "Camera capturing flag is false after start capture call.");

    //destroy manager, not needed anymore
    royale_camera_manager_destroy (man_hnd);
}

void test_level4_tear_down_camera (void)
{
    TEST_PRINT_FUNCTION_NAME;

    status = royale_camera_device_stop_capture (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to stop capturing.");

    bool is_capturing;
    status = royale_camera_device_is_capturing (cam_hnd, &is_capturing);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to check if camera is capturing.");
    TEST_ASSERT_FALSE_MESSAGE (is_capturing, "Camera capturing flag is true after stop capture call.");

    royale_camera_device_destroy (cam_hnd);
}


void test_royaleCAPI_CameraDevice_Level4 (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_INIT;

    RUN_TEST (test_level4_create_manager_and_init_camera);
    RUN_TEST (test_level4_tear_down_camera);

    TEST_MUTEX_DELETE;
}
