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
#include <royaleCAPI.h>
#include <CameraManagerCAPI.h>
#include <TestHelpers.h>
#include <TestCameraDeviceL1.h>
#include <stdlib.h>
#include <string.h>
#include <MsvcMacros.hpp>

#include <sys/stat.h>

static bool received_depth_data            = false;
static bool received_depth_image           = false;
static bool received_exposure              = false;
static bool received_ir_image              = false;
static bool received_spc_data              = false;
static bool received_record_stop           = false;

static bool should_receive_depth_data      = false;
static bool should_receive_depth_image     = false;
static bool should_receive_exposure        = false;
static bool should_receive_ir_image        = false;
static bool should_receive_spc_data        = false;
static bool should_receive_record_stop     = false;

static royale_camera_handle cam_hnd;
static royale_camera_status status;

#define NR_FRAMES_TO_RECORD         10
#define RECORD_FILE_NAME            "test_record.rrf"

static void callback_depth_data (royale_depth_data *data)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_depth_data;
    received_depth_data = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETVOID (should_receive_depth_data, "Received depth data after unregister.");

    // prevent to run the following checks more than once
    if (was_first_callback)
    {
        TEST_CALLBACK_CONDITION_RETVOID (data != NULL, "Received NULL pointer in depth data callback.");
        TEST_CALLBACK_CONDITION_RETVOID (data->points != NULL, "Received NULL pointer in depth data callback.");
        TEST_CALLBACK_CONDITION_RETVOID (0 != data->height, "Received invalid height");
        TEST_CALLBACK_CONDITION_RETVOID (0 != data->width, "Received invalid width");
        uint32_t nr_pixels = (uint32_t) (data->width * data->height);
        TEST_CALLBACK_CONDITION_RETVOID (data->nr_points == nr_pixels, "Invalid number of points received.");
        TEST_CALLBACK_CONDITION_RETVOID (0 != data->nr_exposure_times, "Invalid number of exposure times.");
        TEST_CALLBACK_CONDITION_RETVOID (data->exposure_times != NULL, "Received NULL pointer in depth data callback.");
        TEST_CALLBACK_CONDITION_RETVOID (0 != data->timestamp, "Invalid time stamp received.");
    }

    TEST_MUTEX_UNLOCK;
}

static void callback_depth_image (royale_depth_image *image)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_depth_image;
    received_depth_image = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETVOID (should_receive_depth_image, "Received depth image after unregister.");

    //prevent to run the following checks more than once
    if (was_first_callback)
    {
        TEST_CALLBACK_CONDITION_RETVOID (NULL != image, "Received NULL pointer in depth image callback.");
        TEST_CALLBACK_CONDITION_RETVOID (NULL != image->cdData, "Received NULL pointer in depth image callback.");
        uint32_t nr_pixels = (uint32_t) (image->width * image->height);
        TEST_CALLBACK_CONDITION_RETVOID (nr_pixels == image->nr_data_entries, "Invalid number of data entries received.");
        TEST_CALLBACK_CONDITION_RETVOID (0 != image->timestamp, "Invalid time stamp received.");
    }

    TEST_MUTEX_UNLOCK;
}

static void callback_ir_image (royale_ir_image *image)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_ir_image;
    received_ir_image = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETVOID (should_receive_ir_image, "Received IR image after unregister.");

    //prevent to run the following checks more than once
    if (was_first_callback)
    {
        TEST_CALLBACK_CONDITION_RETVOID (NULL != image, "Received NULL pointer in IR image callback.");
        TEST_CALLBACK_CONDITION_RETVOID (NULL != image->data, "Received NULL pointer in IR image callback.");
        uint32_t nr_pixels = (uint32_t) (image->width * image->height);
        TEST_CALLBACK_CONDITION_RETVOID (nr_pixels == image->nr_data_entries, "Invalid number of data entries received.");
        TEST_CALLBACK_CONDITION_RETVOID (0 != image->timestamp, "Invalid time stamp received.");
    }

    TEST_MUTEX_UNLOCK;
}

static void callback_spc_data (royale_sparse_point_cloud *spc)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_spc_data;
    received_spc_data = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETVOID (should_receive_spc_data, "Received SPC data after unregister.");

    //prevent to run the following checks more than once
    if (was_first_callback)
    {
        TEST_CALLBACK_CONDITION_RETVOID (NULL != spc, "Received NULL pointer in SPC callback.");
        TEST_CALLBACK_CONDITION_RETVOID (NULL != spc->xyzcPoints, "Received NULL pointer in SPC callback.");
        TEST_CALLBACK_CONDITION_RETVOID (0 != spc->timestamp, "Invalid time stamp received.");
    }

    TEST_MUTEX_UNLOCK;
}

static void callback_exposure (uint32_t exposureTime)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_exposure;
    received_exposure = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETVOID (should_receive_exposure, "Received exposure callback (_v220) after unregister.");

    //prevent to run the following checks more than once
    if (was_first_callback)
    {
        TEST_CALLBACK_CONDITION_RETVOID (0 != exposureTime, "Received 0ms exposure time in exposure callback.");
    }

    TEST_MUTEX_UNLOCK;
}

static void callback_record_stop (uint32_t nr_frames)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;

    const bool was_first_callback = !received_record_stop;
    received_record_stop = true;

    // detect callback after unregister
    TEST_CALLBACK_CONDITION_RETVOID (should_receive_record_stop, "Received record stop data after unregister.");

    //prevent to run the following checks more than once
    if (was_first_callback)
    {
        TEST_CALLBACK_CONDITION_RETVOID (NR_FRAMES_TO_RECORD == nr_frames, "Wrong number of frames recorded.");

        struct stat st;
        int file_status = stat (RECORD_FILE_NAME, &st);
        TEST_CALLBACK_CONDITION_RETVOID (0 == file_status, "Record file does not exist.");
    }

    TEST_MUTEX_UNLOCK;
}

void test_level1_create_manager_and_init_camera (void)
{
    TEST_PRINT_FUNCTION_NAME;

    royale_cam_manager_hnd man_hnd = royale_camera_manager_create ();
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, man_hnd, "Failed to create camera manager LEVEL 1.");

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

    //destroy manager, not needed anymore
    royale_camera_manager_destroy (man_hnd);

    // initialize camera
    status = royale_camera_device_initialize (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to initialize camera.");

    // check access level
    royale_camera_access_level level;
    status = royale_camera_device_get_access_level (cam_hnd, &level);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get access level.");
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_ACCESS_LEVEL1, level, "Camera has wrong access level.");

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
}

void test_level1_tear_down_camera (void)
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

void test_level1_camera_info (void)
{
    TEST_PRINT_FUNCTION_NAME;

    // get camera ID
    {
        char *cam_id;
        status = royale_camera_device_get_id (cam_hnd, &cam_id);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get camera ID.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (NULL, cam_id, "Failed to get camera ID.");
        TEST_ASSERT_EQUAL_MESSAGE (19, strlen (cam_id), "Wrong format for camera ID.");
        free ( (void *) cam_id);
    }

    // get camera name
    {
        char *cam_name;
        status = royale_camera_device_get_camera_name (cam_hnd, &cam_name);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get camera name.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (NULL, cam_name, "Failed to get camera name.");
        free ( (void *) cam_name);
    }

    // get camera info
    {
        uint32_t nr_info_entries = UINT32_MAX;
        royale_pair_string_string *cam_info;
        status = royale_camera_device_get_camera_info (cam_hnd, &cam_info, &nr_info_entries);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get camera info.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (UINT32_MAX, nr_info_entries, "Failed to get number of camera info entries.");
        if (nr_info_entries > 0)
        {
            royale_free_pair_string_string_array (&cam_info, nr_info_entries);
        }
    }

    // sensor dimensions
    {
        uint16_t max_width;
        uint16_t max_height;

        status = royale_camera_device_get_max_sensor_width (cam_hnd, &max_width);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get max sensor width.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, max_width, "Failed to get max sensor width");

        status = royale_camera_device_get_max_sensor_height (cam_hnd, &max_height);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get max sensor height.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, max_height, "Failed to get max sensor height");
    }

    // lens parameters
    {
        royale_lens_parameters l_params;
        status = royale_camera_device_get_lens_parameters (cam_hnd, &l_params);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get lens parameters.");

        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.distortionTangential.first, "Invalid tangential distortion data.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.distortionTangential.second, "Invalid tangential distortion data.");

        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.focalLength.first , "Invalid focal length data.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.focalLength.second, "Invalid focal length data.");

        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.principalPoint.first, "Invalid principal point data.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.principalPoint.second, "Invalid principal point data.");

        TEST_ASSERT_EQUAL_MESSAGE (3, l_params.distortionRadial.nrOfValues, "Invalid number of parameters for radial distortion.");
        int i;
        for (i = 0; i < 3; i++)
        {
            TEST_ASSERT_NOT_EQUAL_MESSAGE (0, l_params.distortionRadial.values[i], "Invalid radial distortion data.");
        }
        royale_free_lens_parameters (&l_params);
    }
}

void test_level1_use_cases (void)
{
    TEST_PRINT_FUNCTION_NAME;

    uint32_t nr_use_cases = UINT32_MAX;
    char **use_cases;
    status = royale_camera_device_get_use_cases (cam_hnd, &use_cases, &nr_use_cases);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get use cases.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (UINT32_MAX, nr_use_cases, "Failed to get number of use cases.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (NULL, use_cases, "Failed to get use cases.");

    uint32_t i;
    //try to set each use case and check if it is set correctly
    for (i = 0; i < nr_use_cases; i++)
    {
        status = royale_camera_device_set_use_case (cam_hnd, use_cases[i]);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set use case.");
        char *current_use_case = NULL;
        status = royale_camera_device_get_current_use_case (cam_hnd, &current_use_case);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get current use case.");
        TEST_ASSERT_EQUAL_MESSAGE (0, strcmp (use_cases[i], current_use_case), "Use case was not set correctly.");
        royale_free_string (current_use_case);
    }

    //reset to first use case
    status = royale_camera_device_set_use_case (cam_hnd, use_cases[0]);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set use case.");

    royale_free_string_array (use_cases, nr_use_cases);
}

void test_level1_exposure (void)
{
    TEST_PRINT_FUNCTION_NAME;

    // set exposure time
    {
        uint32_t lower_limit = UINT32_MAX;
        uint32_t upper_limit = 0;

        // get exposure time to limits
        status = royale_camera_device_get_exposure_limits (cam_hnd, &lower_limit, &upper_limit);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get exposure limits.");
        TEST_ASSERT_MESSAGE (lower_limit != UINT32_MAX && upper_limit != 0, "Failed to get exposure limits.");

        // set exposure time to limits
        status = royale_camera_device_set_exposure_time (cam_hnd, lower_limit);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure time to lower limit.");

        TEST_SLEEP_MS (2000);

        status = royale_camera_device_set_exposure_time (cam_hnd, upper_limit);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure time to upper limit.");
    }

    // set exposure mode
    {
        royale_exposure_mode mode;

        // AUTOMATIC
        status = royale_camera_device_set_exposure_mode (cam_hnd, ROYALE_EXPOSURE_AUTOMATIC);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure mode to AUTOMATIC.");
        status = royale_camera_device_get_exposure_mode2 (cam_hnd, &mode);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get exposure mode.");
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_EXPOSURE_AUTOMATIC, mode, "Exposure mode has not been set to AUTOMATIC.");

        // MANUAL
        status = royale_camera_device_set_exposure_mode (cam_hnd, ROYALE_EXPOSURE_MANUAL);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure mode to MANUAL.");
        status = royale_camera_device_get_exposure_mode2 (cam_hnd, &mode);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get exposure mode.");
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_EXPOSURE_MANUAL, mode, "Exposure mode has not been set to MANUAL.");
    }

    // set exposure listener
    {
        status = royale_camera_device_register_exposure_listener (cam_hnd, callback_exposure);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure listener");

        status = royale_camera_device_unregister_exposure_listener (cam_hnd);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister exposure listener");
    }
}

// A simplification for test_level1_exposure_callback, not used in test_level1_exposure because
// the error messages wouldn't be as easy to interpret.
//
// If changing to MANUAL mode, this function will block before returning, so that the caller can
// immediately call royale_camera_device_set_exposure_time without getting a BUSY error.
// Prerequisite: the depth imager listener is registered and expecting callbacks
static void set_and_check_exposure_mode (royale_camera_handle handle, const royale_exposure_mode targetMode)
{
    royale_exposure_mode mode;

    status = royale_camera_device_set_exposure_mode (cam_hnd, targetMode);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure mode to targetMode.");
    status = royale_camera_device_get_exposure_mode2 (cam_hnd, &mode);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get exposure mode.");
    TEST_ASSERT_EQUAL_MESSAGE (targetMode, mode, "Exposure mode has not been set to targetMode.");

    if (targetMode == ROYALE_EXPOSURE_MANUAL)
    {
        // Assuming the use case is not a mixed-mode one, the worst case should be 3 frames:
        // one received frame already in C++ callback layer,
        // one in the processing,
        // and one before the imager's safe reconfig happens.
        TEST_MUTEX_LOCK;
        received_depth_image = false;
        TEST_MUTEX_UNLOCK;
        status = wait_for_callback_called_count (&received_depth_image, 3);
        TEST_ASSERT_MESSAGE (status, "No/few callbacks after setting the exposure mode.");
    }
}

// When this function is called, there will be at least one callback to the exposure listener
static void trigger_exposure_callback (void)
{
    // get exposure limits
    uint32_t lower_limit = UINT32_MAX;
    uint32_t upper_limit = 0;
    status = royale_camera_device_get_exposure_limits (cam_hnd, &lower_limit, &upper_limit);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get exposure limits.");
    TEST_ASSERT_MESSAGE (lower_limit != UINT32_MAX && upper_limit != 0, "Failed to get exposure limits.");
    TEST_ASSERT_MESSAGE (lower_limit != upper_limit, "Use case has a fixed exposure");

    // Callbacks only happen when the autoexposure changes the exposure.  Set exposure time to
    // one limit, give time for the autoexposure to happen, set the exposure to the other limit
    // and give time for the autoexposure to happen.  Then check there was at least one
    // callback.
    //
    // But to set the exposure without getting a BUSY error, the exposure needs to be in MANUAL
    // mode.
    //
    // Also the autoexposure only runs when data is being processed, which only happens when a
    // data listener is registered.

    // set depth image callback; if there's no listener then the camera doesn't autofocus
    TEST_MUTEX_LOCK;
    should_receive_depth_image = true;
    received_depth_image = false;
    TEST_MUTEX_UNLOCK;

    status = royale_camera_device_register_depth_image_listener (cam_hnd, callback_depth_image);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register depth image listener.");

    set_and_check_exposure_mode (cam_hnd, ROYALE_EXPOSURE_MANUAL);
    status = royale_camera_device_set_exposure_time (cam_hnd, lower_limit);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure time to lower limit.");
    set_and_check_exposure_mode (cam_hnd, ROYALE_EXPOSURE_AUTOMATIC);

    // the callback might happen during this sleep
    wait_for_callback_called_count_timeout (&received_exposure, 1, 2000);

    set_and_check_exposure_mode (cam_hnd, ROYALE_EXPOSURE_MANUAL);
    status = royale_camera_device_set_exposure_time (cam_hnd, lower_limit);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure time to lower limit.");
    set_and_check_exposure_mode (cam_hnd, ROYALE_EXPOSURE_AUTOMATIC);

    // the callback might happen during this sleep
    wait_for_callback_called_count_timeout (&received_exposure, 1, 2000);

    // set exposure mode to manual, to prevent unexpected changes in other tests
    // Must be done before unregistering the depth listener (prerequisite for
    // set_and_check_exposure_mode).
    set_and_check_exposure_mode (cam_hnd, ROYALE_EXPOSURE_MANUAL);

    // unregister the depth listener
    status = royale_camera_device_unregister_depth_image_listener (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister depth image listener.");
    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_depth_image = false;
    TEST_MUTEX_UNLOCK;
}

void test_level1_exposure_callback (void)
{
    TEST_PRINT_FUNCTION_NAME;

    // To run this on the Long Term Support branch, cherry-pick commit 22465b5, and it makes sense
    // to cherry-pick 8995bb7 too. Without that it crashes, so instead just fail the test.
    if (ROYALE_VERSION_MAJOR == 2)
    {
        const int minorWhenBugFixed = 65535; // change when fixed
        TEST_ASSERT_MESSAGE (minorWhenBugFixed <= ROYALE_VERSION_MINOR, "Until ROYAL-1805 is backported, this will crash");
    }

    TEST_MUTEX_LOCK;
    should_receive_exposure = true;
    received_exposure = false;
    TEST_CLEAR_ASYNC_ERROR_FLAG;
    TEST_MUTEX_UNLOCK;

    // test v220 exposure listener
    {
        status = royale_camera_device_register_exposure_listener (cam_hnd, callback_exposure);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set exposure listener");
        trigger_exposure_callback();
        TEST_ASSERT_TRUE_MESSAGE (received_exposure, "Didn't receive a v220 ROYALE_EXPOSURE_CALLBACK");
        status = royale_camera_device_unregister_exposure_listener (cam_hnd);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister exposure listener");
    }

    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_exposure = false;
    TEST_MUTEX_UNLOCK;
    TEST_ASSERT_NO_ASYNC_ERROR;
}

void test_level1_depth_data_callback (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;
    should_receive_depth_data = true;
    received_depth_data = false;
    TEST_CLEAR_ASYNC_ERROR_FLAG;
    TEST_MUTEX_UNLOCK;

    // set depth data callback
    status = royale_camera_device_register_data_listener (cam_hnd, callback_depth_data);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register depth data listener.");

    // wait for callback data checks
    bool called = wait_for_callback_called (&received_depth_data);

    status = royale_camera_device_unregister_data_listener (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister depth data listener.");

    TEST_ASSERT_TRUE_MESSAGE (called, "Depth data callback was not called.");
    TEST_ASSERT_NO_ASYNC_ERROR;

    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_depth_data = false;
    TEST_MUTEX_UNLOCK;
}

void test_level1_depth_image_callback (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;
    should_receive_depth_image = true;
    received_depth_image = false;
    TEST_CLEAR_ASYNC_ERROR_FLAG;
    TEST_MUTEX_UNLOCK;

    // set depth image callback
    status = royale_camera_device_register_depth_image_listener (cam_hnd, callback_depth_image);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register depth image listener.");

    // wait for callback image checks
    bool called = wait_for_callback_called (&received_depth_image);

    status = royale_camera_device_unregister_depth_image_listener (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister depth image listener.");

    TEST_ASSERT_TRUE_MESSAGE (called, "Depth image callback was not called.");
    TEST_ASSERT_NO_ASYNC_ERROR;

    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_depth_image = false;
    TEST_MUTEX_UNLOCK;
}

void test_level1_ir_image_callback (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;
    should_receive_ir_image = true;
    received_ir_image = false;
    TEST_CLEAR_ASYNC_ERROR_FLAG;
    TEST_MUTEX_UNLOCK;

    // set depth image callback
    status = royale_camera_device_register_ir_image_listener (cam_hnd, callback_ir_image);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register IR image listener.");

    // wait for callback image checks
    bool called = wait_for_callback_called (&received_ir_image);

    status = royale_camera_device_unregister_ir_image_listener (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister IR image listener.");

    TEST_ASSERT_TRUE_MESSAGE (called, "IR image callback was not called.");
    TEST_ASSERT_NO_ASYNC_ERROR;

    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_ir_image = false;
    TEST_MUTEX_UNLOCK;
}

void test_level1_spc_callback (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_LOCK;
    should_receive_spc_data = true;
    received_spc_data = false;
    TEST_CLEAR_ASYNC_ERROR_FLAG;
    TEST_MUTEX_UNLOCK;

    // set depth image callback
    status = royale_camera_device_register_spc_listener (cam_hnd, callback_spc_data);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register SPC listener.");

    // wait for callback image checks
    bool called = wait_for_callback_called (&received_spc_data);

    status = royale_camera_device_unregister_spc_listener (cam_hnd);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister SPC listener.");

    TEST_ASSERT_TRUE_MESSAGE (called, "SPC callback was not called.");
    TEST_ASSERT_NO_ASYNC_ERROR;

    TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
    TEST_MUTEX_LOCK;
    should_receive_spc_data = false;
    TEST_MUTEX_UNLOCK;
}

void test_level1_recording (void)
{
    TEST_PRINT_FUNCTION_NAME;

    // check record stop listener
    {
        TEST_MUTEX_LOCK;
        should_receive_record_stop = true;
        received_record_stop = false;
        TEST_CLEAR_ASYNC_ERROR_FLAG;
        TEST_MUTEX_UNLOCK;

        status = royale_camera_device_register_record_stop_listener (cam_hnd, callback_record_stop);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to register record stop listener.");

        status = royale_camera_device_start_recording (cam_hnd, RECORD_FILE_NAME, NR_FRAMES_TO_RECORD, 0, 0);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to start recording.");

        TEST_SLEEP_MS (2000);

        // wait for callback image checks
        bool called = wait_for_callback_called (&received_record_stop);
        FILE_UNLINK (RECORD_FILE_NAME);

        status = royale_camera_device_unregister_record_stop_listener (cam_hnd);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to unregister record stop listener.");

        TEST_ASSERT_TRUE_MESSAGE (called, "Record stop callback was not called.");
        TEST_ASSERT_NO_ASYNC_ERROR;

        TEST_SLEEP_MS (WAIT_MS_AFTER_UNREGISTER);
        TEST_MUTEX_LOCK;
        should_receive_record_stop = false;
        TEST_MUTEX_UNLOCK;
    }

    // check manual stop of recording
    {
        status = royale_camera_device_start_recording (cam_hnd, RECORD_FILE_NAME, 0, 0, 0);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to start recording.");

        TEST_SLEEP_MS (2000);

        status = royale_camera_device_stop_recording (cam_hnd);
        FILE_UNLINK (RECORD_FILE_NAME);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to stop recording.");
    }
}

void test_level1_frame_rate (void)
{
    TEST_PRINT_FUNCTION_NAME;

    uint16_t frame_rate;
    uint16_t frame_rate_max;

    status = royale_camera_device_get_frame_rate (cam_hnd, &frame_rate);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get frame rate.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, frame_rate, "Invalid framerate.");

    status = royale_camera_device_get_max_frame_rate (cam_hnd, &frame_rate_max);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get maximum frame rate.");
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, frame_rate_max, "Invalid max framerate.");

    status = royale_camera_device_set_frame_rate (cam_hnd, (uint16_t) (frame_rate_max - 1));
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to set frame rate.");

    status = royale_camera_device_get_frame_rate (cam_hnd, &frame_rate);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get frame rate.");
    TEST_ASSERT_EQUAL_MESSAGE ( (uint16_t) (frame_rate_max - 1), frame_rate, "Wrong frame rate was set.");

    status = royale_camera_device_set_frame_rate (cam_hnd, (uint16_t) (frame_rate_max + 1));
    TEST_ASSERT_NOT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Possible to set frame rate beyond limit.");
}

void test_royaleCAPI_CameraDevice_Level1 (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_INIT;

    RUN_TEST (test_level1_create_manager_and_init_camera);
    RUN_TEST (test_level1_camera_info);
    RUN_TEST (test_level1_use_cases);
    RUN_TEST (test_level1_exposure);
    RUN_TEST (test_level1_exposure_callback);
    RUN_TEST (test_level1_depth_data_callback);
    RUN_TEST (test_level1_depth_image_callback);
    RUN_TEST (test_level1_ir_image_callback);
    RUN_TEST (test_level1_spc_callback);
    RUN_TEST (test_level1_recording);
    RUN_TEST (test_level1_frame_rate);
    RUN_TEST (test_level1_tear_down_camera);

    TEST_MUTEX_DELETE;
}
