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
#include <stdlib.h>

#include <royaleCAPI.h>
#include <ExtendedDataCAPI.h>
#include <CameraManagerCAPI.h>
#include <TestHelpers.h>
#include <TestCameraDeviceL2.h>

#include <MsvcMacros.hpp>

royale_camera_handle cam_hnd;
royale_camera_status status;

static bool received_extended_data         = false;
static bool should_receive_extended_data   = false;

ROYALE_EXTENDED_DATA_CALLBACK callback_extended (royale_extended_data *data)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_extended_data;
    received_extended_data = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETNULL (should_receive_extended_data, "Received extended data after unregister.");

    // prevent to run the following checks more than once
    if (!was_first_callback)
    {
        TEST_MUTEX_UNLOCK;
        return (void *) 0;
    }

    TEST_CALLBACK_CONDITION_RETNULL (NULL != data, "Received NULL pointer in extended data callback.");
    if (data->has_depth_data)
    {
        royale_depth_data d_data = * (data->depth_data);

        TEST_CALLBACK_CONDITION_RETNULL (NULL != d_data.points, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != d_data.height, "Received invalid height");
        TEST_CALLBACK_CONDITION_RETNULL (0 != d_data.width, "Received invalid width");
        uint32_t nr_pixels = (uint32_t) (d_data.width * d_data.height);
        TEST_CALLBACK_CONDITION_RETNULL (d_data.nr_points == nr_pixels, "Invalid number of points received.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != d_data.nr_exposure_times, "Invalid number of exposure times.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != d_data.exposure_times, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != d_data.timestamp, "Invalid time stamp received.");
    }

    if (data->has_intermediate_data)
    {
        royale_intermediate_data i_data = * (data->intermediate_data);
        TEST_CALLBACK_CONDITION_RETNULL (NULL != i_data.points, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != i_data.height, "Received invalid height");
        TEST_CALLBACK_CONDITION_RETNULL (0 != i_data.width, "Received invalid width");
        uint32_t nr_pixels = (uint32_t) (i_data.width * i_data.height);
        TEST_CALLBACK_CONDITION_RETNULL (i_data.nr_points == nr_pixels, "Invalid number of points received.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != &i_data.points[nr_pixels - 1], "Received data length shorter than reported.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != i_data.nr_exposure_times, "Invalid number of exposure times.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != i_data.nr_mod_frequencies, "Invalid number of modulation frequencies.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != i_data.exposure_times, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != i_data.modulation_frequencies, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != i_data.nr_frequencies, "Invalid number of frequencies.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != i_data.timestamp, "Invalid time stamp received.");
    }

    if (data->has_raw_data)
    {
        royale_raw_data r_data = * (data->raw_data);
        TEST_CALLBACK_CONDITION_RETNULL (NULL != r_data.raw_data, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != r_data.height, "Received invalid height");
        TEST_CALLBACK_CONDITION_RETNULL (0 != r_data.width, "Received invalid width");
        uint32_t nr_pixels = (uint32_t) (r_data.width * r_data.height);
        TEST_CALLBACK_CONDITION_RETNULL (r_data.nr_data_per_raw_frame == nr_pixels, "Invalid number of points received.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != &r_data.raw_data[r_data.nr_raw_frames - 1][nr_pixels - 1], "Received data length shorter than reported.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != r_data.nr_exposure_times, "Invalid number of exposure times.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != r_data.nr_mod_frequencies, "Invalid number of modulation frequencies.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != r_data.exposure_times, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (NULL != r_data.modulation_frequencies, "Received NULL pointer in extended data callback.");
        TEST_CALLBACK_CONDITION_RETNULL (0 != r_data.timestamp, "Invalid time stamp received.");
    }

    TEST_MUTEX_UNLOCK;

    return (void *) 0;
}

void test_level2_create_manager_and_init_camera (void)
{
    TEST_PRINT_FUNCTION_NAME;

    royale_cam_manager_hnd man_hnd = royale_camera_manager_create_with_code (ROYALE_ACCESS_CODE_LEVEL2);
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, man_hnd, "Failed to create camera manager LEVEL 2.");

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

    // initialize camera
    status = royale_camera_device_initialize (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to initialize camera.");

    // check access level
    royale_camera_access_level level;
    status = royale_camera_device_get_access_level (cam_hnd, &level);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get access level.");
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_ACCESS_LEVEL2, level, "Camera has wrong access level.");

    // check flags
    bool is_connected;
    bool is_calibrated;
    bool is_capturing;

    status = royale_camera_device_is_connected (cam_hnd, &is_connected);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "failed to get connected flag.");
    TEST_ASSERT_TRUE_MESSAGE (is_connected, "Camera connected flag is invalid.");

    status = royale_camera_device_is_calibrated (cam_hnd, &is_calibrated);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "failed to get calibrated flag.");
    TEST_ASSERT_TRUE_MESSAGE (is_calibrated, "Camera calibrated flag is invalid");

    status = royale_camera_device_is_capturing (cam_hnd, &is_capturing);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "failed to get capturing flag.");
    TEST_ASSERT_FALSE_MESSAGE (is_capturing, "Camera capturing flag is true before start capture call.");

    // start capture
    status = royale_camera_device_start_capture (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to start capturing.");

    status = royale_camera_device_is_capturing (cam_hnd, &is_capturing);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "failed to get capturing flag.");
    TEST_ASSERT_TRUE_MESSAGE (is_capturing, "Camera capturing flag is false after start capture call.");

    //destroy manager, not needed anymore
    royale_camera_manager_destroy (man_hnd);
}

void test_level2_tear_down_camera (void)
{
    TEST_PRINT_FUNCTION_NAME;

    status = royale_camera_device_stop_capture (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to stop capturing.");

    bool is_capturing;
    status = royale_camera_device_is_capturing (cam_hnd, &is_capturing);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "failed to get capturing flag.");
    TEST_ASSERT_FALSE_MESSAGE (is_capturing, "Camera capturing flag is true after stop capture call.");

    royale_camera_device_destroy (cam_hnd);
}

void test_level2_exposure_time (void)
{
    TEST_PRINT_FUNCTION_NAME;

    uint32_t lower_limit = 0;
    uint32_t upper_limit = 0;

    status = royale_camera_device_get_exposure_limits (cam_hnd, &lower_limit, &upper_limit);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "failed to get exposure limits.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, lower_limit, "Invalid exposure limit.");

    uint32_t exposure_times[5];
    int i;
    for (i = 0; i < 5; i++)
    {
        exposure_times[i] = lower_limit;
    }

    status = royale_camera_device_set_exposure_times (cam_hnd, exposure_times, 5);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure times.");
}

void test_level2_processing_parameters (void)
{
    TEST_PRINT_FUNCTION_NAME;

    royale_processing_parameter *p_params = 0;
    uint32_t nr_params = 0;

    // With Royale v3.4.0 or before, all the flags were always present. From v3.5.0 onwards some may
    // be omitted; there is no defined list of these, but for testing the API it's useful to
    // sanity-check the number.  Therefore a list is kept here.
    const royale_processing_flag optional_flags[] = {ROYALE_PROC_FLAG_USE_REMOVE_STRAYLIGHT_BOOL};
    const size_t nr_optional_flags = sizeof (optional_flags) / sizeof (optional_flags[0]);

    status = royale_camera_device_get_processing_parameters (cam_hnd, &p_params, &nr_params);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get processing parameters.");
    TEST_ASSERT_TRUE_MESSAGE (nr_params <= ROYALE_PROC_FLAG_NUM_FLAGS, "Invalid processing parameter count (too many).");
    TEST_ASSERT_TRUE_MESSAGE (nr_params + nr_optional_flags >= ROYALE_PROC_FLAG_NUM_FLAGS, "Invalid processing parameter count (too few).");

    status = royale_camera_device_set_processing_parameters (cam_hnd, &p_params, nr_params);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set processing parameters.");

    size_t i = 0;
    for (i = 0; i < ROYALE_PROC_FLAG_NUM_FLAGS; i++)
    {
        size_t j = 0;
        bool optional = false;
        for (j = 0; j < nr_optional_flags; j++)
        {
            if (i == optional_flags [j])
            {
                optional = true;
                break;
            }
        }

        if (!optional)
        {
            bool found = false;
            for (j = 0; j < nr_params; j++)
            {
                if (p_params[j].flag == i)
                {
                    found = true;
                    break;
                }
            }
            TEST_ASSERT_TRUE_MESSAGE (found, "Non-optional processing flag not found");
        }
    }

    char *flagName = NULL;
    royale_proc_flag_get_flag_name (&flagName, p_params[0].flag);
    TEST_ASSERT_NOT_NULL_MESSAGE (flagName, "Failed to get processing flag name.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, strlen (flagName), "Failed to get processing flag name.");
    free ( (void *) flagName);
}

void test_level2_extended_data_callback (void)
{
    TEST_PRINT_FUNCTION_NAME;
    TEST_MUTEX_LOCK;
    should_receive_extended_data = true;
    received_extended_data = false;
    TEST_CLEAR_ASYNC_ERROR_FLAG;
    TEST_MUTEX_UNLOCK;

    status = royale_camera_device_register_extended_data_listener (cam_hnd, (ROYALE_EXTENDED_DATA_CALLBACK) callback_extended);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register extended data listener.");

    // wait for callback image checks
    bool called = wait_for_callback_called (&received_extended_data);

    status = royale_camera_device_unregister_extended_data_listener (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister extended data listener.");

    TEST_ASSERT_TRUE_MESSAGE (called, "extended data callback was not called.");
    TEST_ASSERT_NO_ASYNC_ERROR;

    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_extended_data = false;
    TEST_MUTEX_UNLOCK;

    status = royale_camera_device_set_callback_data (cam_hnd, ROYALE_CB_DATA_DEPTH);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set callback data.");

    status = royale_camera_device_set_callback_dataU16 (cam_hnd, (uint16_t) ROYALE_CB_DATA_DEPTH);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set callback data.");
}

void test_level2_calibration_data (void)
{
    uint32_t nr_calib_data;
    uint8_t *calib_data = NULL;

    // We don't test writing calibration data, because an automated test is not expected to
    // overwrite data on the device.
    // status = royale_camera_device_write_calibration_to_flash (cam_hnd);
    // TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to write calibration data to flash.");

    status = royale_camera_device_get_calibration_data (cam_hnd, &calib_data, &nr_calib_data);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get calibration data.");

    status = royale_camera_device_set_calibration_data (cam_hnd, &calib_data, nr_calib_data);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set calibration data.");

    free (calib_data);
}

void test_royaleCAPI_CameraDevice_Level2 (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_INIT;

    RUN_TEST (test_level2_create_manager_and_init_camera);
    RUN_TEST (test_level2_exposure_time);
    RUN_TEST (test_level2_processing_parameters);
    RUN_TEST (test_level2_extended_data_callback);
    RUN_TEST (test_level2_calibration_data);
    RUN_TEST (test_level2_tear_down_camera);

    TEST_MUTEX_DELETE;
}
