/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

/****************************************************************************\
* These functions will only be used internally for the .NET wrapper.
* They are hidden from the publicly available CAPI.
\****************************************************************************/

#pragma once

#include <CameraDeviceCAPI.h>
#include <DefinitionsCAPI.h>
#include <StatusCAPI.h>
#include <stdint.h>

ROYALE_CAPI_LINKAGE_TOP

ROYALE_CAPI typedef void (*PROCESSING_PARAMETER_CALLBACK) (uint32_t *, uint32_t);

ROYALE_CAPI typedef void (*CALIBRATION_DATA_CALLBACK) (uint8_t *, uint32_t);

ROYALE_CAPI royale_camera_status royale_camera_device_managed_set_proc_params (royale_camera_handle handle,
        royale_stream_id streamId, uint32_t *processingParameterArray, uint32_t totalLength);

ROYALE_CAPI royale_camera_status royale_camera_device_managed_get_proc_params (royale_camera_handle handle,
        royale_stream_id streamId, PROCESSING_PARAMETER_CALLBACK callback);

ROYALE_CAPI royale_camera_status royale_camera_device_managed_get_calib_data (royale_camera_handle handle,
        CALIBRATION_DATA_CALLBACK callback);

ROYALE_CAPI_LINKAGE_BOTTOM
