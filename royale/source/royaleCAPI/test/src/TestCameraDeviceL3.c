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
#include <CameraManagerCAPI.h>
#include <TestHelpers.h>
#include <TestCameraDeviceL3.h>

royale_camera_handle cam_hnd;
royale_camera_status status;

void test_level3_create_manager_and_init_camera (void)
{
    TEST_PRINT_FUNCTION_NAME;

    royale_cam_manager_hnd man_hnd = royale_camera_manager_create_with_code (ROYALE_ACCESS_CODE_LEVEL3);
    TEST_ASSERT_NOT_EQUAL_MESSAGE (0, man_hnd, "Failed to create camera manager LEVEL 3.");

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
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_ACCESS_LEVEL3, level, "Camera has wrong access level.");

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

void test_level3_tear_down_camera (void)
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

void test_level3_duty_cycle (void)
{
    TEST_PRINT_FUNCTION_NAME;

    status = royale_camera_device_set_duty_cycle (cam_hnd, 0.5, 0);
    TEST_ASSERT_MESSAGE (
        status == ROYALE_STATUS_SUCCESS || status == ROYALE_STATUS_DUTYCYCLE_NOT_SUPPORTED,
        "Failed to set duty cycle.");
}

void test_level3_register_write_read (void)
{
    TEST_PRINT_FUNCTION_NAME;

    // This writes to a register (0xb06e) which on a pico flexx is used for the unused CSI-2 port,
    // check that we're only running on a camera that we know.
    bool ok_to_test_write = false;
    char *cam_name;
    status = royale_camera_device_get_camera_name (cam_hnd, &cam_name);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to get camera name.");

    if (strcmp (cam_name, "PICOFLEXX") == 0)
    {
        ok_to_test_write = true;
    }
    royale_free_string (cam_name);

    uint16_t expected_value = 0x300;

    uint32_t nr_registers = 1;
    royale_pair_string_uint64 *registers = (royale_pair_string_uint64 *) malloc (sizeof (royale_pair_string_uint64));
    registers[0].first = "0xb06e";
    registers[0].second = expected_value;

    // write one register
    if (ok_to_test_write)
    {
        status = royale_camera_device_write_registers (cam_hnd, &registers, nr_registers);
        TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to write registers.");
    }

    // read the register
    registers[0].second = 0x0000;
    status = royale_camera_device_read_registers (cam_hnd, &registers, nr_registers);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to read registers.");
    if (ok_to_test_write)
    {
        // check that we read what we wrote
        TEST_ASSERT_EQUAL_MESSAGE (expected_value, registers[0].second, "Register value was not set correctly.");
    }
    else
    {
        // we don't know what the value should be, instead change expected_value for the next test
        expected_value = (uint16_t) registers[0].second;
    }

    // read it again, also testing that having a larger-than-uint16_t value in registers[0].second
    // doesn't trigger a failing narrow_cast in Royale
    registers[0].second = UINT64_MAX;
    status = royale_camera_device_read_registers (cam_hnd, &registers, nr_registers);
    TEST_ASSERT_EQUAL_MESSAGE (ROYALE_STATUS_SUCCESS, status, "Failed to read registers.");

    TEST_ASSERT_EQUAL_MESSAGE (expected_value, registers[0].second, "Register reading was different.");
}

void test_level3_lens_center (void)
{
    TEST_PRINT_FUNCTION_NAME;

    uint16_t cx, cy;
    uint16_t cx2, cy2;

    status = royale_camera_device_set_callback_data (cam_hnd, ROYALE_CB_DATA_RAW);
    TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_SUCCESS, "Failed to set callback data.");

    status = royale_camera_device_get_lens_center (cam_hnd, &cx, &cy);
    TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_SUCCESS, "Failed to retrieve lens center.");

    uint16_t width, height;
    status = royale_camera_device_get_max_sensor_width (cam_hnd, &width);
    TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_SUCCESS, "Failed to retrieve sensor width.");

    status = royale_camera_device_get_max_sensor_height (cam_hnd, &height);
    TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_SUCCESS, "Failed to retrieve sensor height.");

    status = royale_camera_device_shift_lens_center (cam_hnd, -16, 1);

    if (width == 352 &&
            height == 287)
    {
        // we can't shift the lens center for full frame read out
        TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_INVALID_VALUE, "Succeeded to shift lens center, on a device where that should be impossible.");
        return;
    }
    else
    {
        TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_SUCCESS, "Failed to shift lens center.");
    }

    status = royale_camera_device_get_lens_center (cam_hnd, &cx2, &cy2);
    TEST_ASSERT_MESSAGE (status == ROYALE_STATUS_SUCCESS, "Failed to retrieve lens center.");

    TEST_ASSERT_EQUAL_MESSAGE (cx - 16, cx2, "Failed to shift cx.");
    TEST_ASSERT_EQUAL_MESSAGE (cy + 1, cy2, "Failed to shift cy.");
}

void test_royaleCAPI_CameraDevice_Level3 (void)
{
    TEST_PRINT_FUNCTION_NAME;

    TEST_MUTEX_INIT;

    RUN_TEST (test_level3_create_manager_and_init_camera);
    RUN_TEST (test_level3_lens_center);
    RUN_TEST (test_level3_duty_cycle);
    RUN_TEST (test_level3_register_write_read);
    RUN_TEST (test_level3_tear_down_camera);

    TEST_MUTEX_DELETE;
}
