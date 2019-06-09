/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royale/ICameraDevice.hpp>
#include <common/MakeUnique.hpp>
#include <CameraDeviceCAPI.h>
#include <private/CameraDeviceCreateCAPI.hpp>
#include <private/CameraDeviceManagedCAPI.hpp>
#include <private/InstanceManagerCAPI.hpp>
#include <private/DepthDataListenerCAPI.hpp>
#include <private/DepthImageListenerCAPI.hpp>
#include <private/ExtendedDataListenerCAPI.hpp>
#include <private/IRImageListenerCAPI.hpp>
#include <private/SparsePointCloudListenerCAPI.hpp>
#include <private/EventListenerCAPI.hpp>
#include <private/RecordStopListenerCAPI.hpp>
#include <private/ExposureListenerCAPI.hpp>
#include <private/ProcessingParametersConverterCAPI.hpp>
#include <private/HelperFunctionsCAPI.hpp>

#include<map>
#include<mutex>

using namespace royale;

InstanceManager <std::shared_ptr <ICameraDevice>> m_instanceManager (::InstanceManagerType::CAMERA_DEVICE);

std::mutex m_listenerMapLock;
std::map<royale_camera_handle, std::unique_ptr<royale::IDepthDataListener>> m_depthDataListenerMap;
std::map<royale_camera_handle, std::unique_ptr<royale::IDepthImageListener>> m_depthImageListenerMap;
std::map<royale_camera_handle, std::unique_ptr<royale::IEventListener>> m_eventListenerMap;
// both ExposureListenerCAPI and ExposureListener2CAPI use the IExposureListener2 interface
std::map<royale_camera_handle, std::unique_ptr<royale::IExposureListener2>> m_exposureListenerMap;
std::map<royale_camera_handle, std::unique_ptr<royale::IExtendedDataListener>> m_extendedDataListenerMap;
std::map<royale_camera_handle, std::unique_ptr<royale::IIRImageListener>> m_irImageListenerMap;
std::map<royale_camera_handle, std::unique_ptr<royale::IRecordStopListener>> m_recordStopListenerMap;
std::map<royale_camera_handle, std::unique_ptr<royale::ISparsePointCloudListener>> m_spcListenerMap;

royale_camera_handle royale_camera_device_create (std::shared_ptr<royale::ICameraDevice> cameraInstance)
{
    return m_instanceManager.AddInstance (cameraInstance);
}

template<typename T> void delete_previously_created_listener (royale_camera_handle handle, std::map<royale_camera_handle, std::unique_ptr<T>> &listenerMap)
{
    std::lock_guard<std::mutex> lock (m_listenerMapLock);
    auto it = listenerMap.find (handle);
    if (it != listenerMap.end())
    {
        listenerMap.erase (it);
    }
}

ROYALE_CAPI_LINKAGE_TOP

// ----------------------------------------------------------------------------------------------
// Level 1: Standard users (Laser Class 1 guaranteed)
// ----------------------------------------------------------------------------------------------

ROYALE_CAPI void royale_camera_device_destroy_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (void) 0);

    m_instanceManager.DeleteInstance (handle);
}

ROYALE_CAPI royale_camera_status royale_camera_device_initialize_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->initialize();
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_id_v220 (royale_camera_handle handle, char **camera_id)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    String id;
    royale_camera_status status = (royale_camera_status) instance->getId (id);
    if (ROYALE_STATUS_SUCCESS == status)
    {
        HelperFunctionsCAPI::copyRoyaleStringToCString (camera_id, id);
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_camera_name_v220 (royale_camera_handle handle, char **camera_name)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    String name;
    royale_camera_status status = (royale_camera_status) instance->getCameraName (name);
    if (ROYALE_STATUS_SUCCESS == status)
    {
        HelperFunctionsCAPI::copyRoyaleStringToCString (camera_name, name);
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_camera_info_v220 (royale_camera_handle handle, royale_pair_string_string **info, uint32_t *nr_info_entries)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    Vector<Pair<String, String>> camInfo;
    royale_camera_status status = (royale_camera_status) instance->getCameraInfo (camInfo);

    *nr_info_entries = (uint32_t) camInfo.size();

    // malloc even if there are no entries, to let the user be able to call free() without crashing the program
    *info = (royale_pair_string_string *) malloc (sizeof (royale_pair_string_string) * *nr_info_entries);

    for (uint32_t i = 0; i < *nr_info_entries; i++)
    {
        HelperFunctionsCAPI::copyRoyaleStringToCString (& (*info) [i].first, camInfo[i].first);
        HelperFunctionsCAPI::copyRoyaleStringToCString (& (*info) [i].second, camInfo[i].second);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_use_case_v210 (royale_camera_handle handle,
        const char *use_case_name)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setUseCase (royale::String (use_case_name));
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_use_cases_v220 (royale_camera_handle handle, char ***useCases, uint32_t *nr_use_cases)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<String> royaleUseCases;
    royale_camera_status status = (royale_camera_status) instance->getUseCases (royaleUseCases);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *nr_use_cases = (uint32_t) royaleUseCases.size();

        // malloc even if there are no entries, to let the user be able to call free() without crashing the program
        *useCases = (char **) malloc (sizeof (char *) * *nr_use_cases);

        for (uint32_t i = 0; i < *nr_use_cases; i++)
        {
            HelperFunctionsCAPI::copyRoyaleStringToCString (& (*useCases) [i], royaleUseCases.at (i));
        }
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_streams_v300 (royale_camera_handle handle, royale_vector_stream_id *streams)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<StreamId> royaleStreams;

    streams->nrOfValues = 0;
    streams->values = nullptr;

    royale_camera_status status = (royale_camera_status) instance->getStreams (royaleStreams);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        const auto nStreams = (uint32_t) royaleStreams.size();
        streams->nrOfValues = nStreams;
        streams->values = (uint16_t *) malloc (sizeof (uint16_t) * nStreams);
        for (uint32_t i = 0; i < nStreams; ++i)
        {
            streams->values[i] = royaleStreams.at (i);
        }
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_number_of_streams_v330 (royale_camera_handle handle, const char *use_case_name, uint32_t *nr_streams)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->getNumberOfStreams (use_case_name, *nr_streams);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_current_use_case_v220 (royale_camera_handle handle, char **use_case_name)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    String ucName;
    royale_camera_status status = (royale_camera_status) instance->getCurrentUseCase (ucName);
    if (ROYALE_STATUS_SUCCESS == status)
    {
        HelperFunctionsCAPI::copyRoyaleStringToCString (use_case_name, ucName);
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_time_v210 (royale_camera_handle handle,
        uint32_t exposure_time)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setExposureTime (exposure_time);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_time_v300 (royale_camera_handle handle,
        royale_stream_id stream_id, uint32_t exposure_time)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setExposureTime (exposure_time, stream_id);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_mode_v210 (royale_camera_handle handle,
        royale_exposure_mode exposure_mode)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setExposureMode ( (royale::ExposureMode) exposure_mode);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_mode_v300 (royale_camera_handle handle,
        royale_stream_id stream_id, royale_exposure_mode exposure_mode)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setExposureMode ( (royale::ExposureMode) exposure_mode, stream_id);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_exposure_mode_v220 (royale_camera_handle handle,
        royale_exposure_mode *exposure_mode)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    ExposureMode mode;
    royale_camera_status status = (royale_camera_status) instance->getExposureMode (mode);
    if (ROYALE_STATUS_SUCCESS == status)
    {
        *exposure_mode = (royale_exposure_mode) mode;
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_exposure_mode_v300 (royale_camera_handle handle,
        royale_stream_id stream_id, royale_exposure_mode *exposure_mode)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    ExposureMode mode;
    royale_camera_status status = (royale_camera_status) instance->getExposureMode (mode, stream_id);
    if (ROYALE_STATUS_SUCCESS == status)
    {
        *exposure_mode = (royale_exposure_mode) mode;
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_exposure_limits_v220 (royale_camera_handle handle,
        uint32_t *lower_limit, uint32_t *upper_limit)
{
    *lower_limit = 0;
    *upper_limit = 0;

    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    Pair<uint32_t, uint32_t> exLimits;
    royale_camera_status status = (royale_camera_status) instance->getExposureLimits (exLimits);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *lower_limit = exLimits.first;
        *upper_limit = exLimits.second;
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_exposure_limits_v300 (royale_camera_handle handle,
        royale_stream_id stream_id,
        uint32_t *lower_limit, uint32_t *upper_limit)
{
    *lower_limit = 0;
    *upper_limit = 0;

    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    Pair<uint32_t, uint32_t> exLimits;
    royale_camera_status status = (royale_camera_status) instance->getExposureLimits (exLimits, stream_id);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *lower_limit = exLimits.first;
        *upper_limit = exLimits.second;
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_data_listener_v210 (royale_camera_handle handle,
        ROYALE_DEPTH_DATA_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_data_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<DepthDataListenerCAPI> (callback);
        status = (royale_camera_status) instance->registerDataListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_depthDataListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_depth_image_listener_v210 (royale_camera_handle handle,
        ROYALE_DEPTH_IMAGE_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_depth_image_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<DepthImageListenerCAPI> (callback);
        status = (royale_camera_status) instance->registerDepthImageListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_depthImageListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_ir_image_listener_v210 (royale_camera_handle handle,
        ROYALE_IR_IMAGE_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_ir_image_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<IRImageListenerCAPI> (callback);
        status = (royale_camera_status) instance->registerIRImageListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_irImageListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_spc_listener_v210 (royale_camera_handle handle,
        ROYALE_SPC_DATA_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_spc_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<SparsePointCloudListenerCAPI> (callback);
        status = (royale_camera_status) instance->registerSparsePointCloudListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_spcListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_event_listener_v210 (royale_camera_handle handle,
        ROYALE_EVENT_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_event_listener_v210 (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<EventListenerCAPI> (callback);
        status = (royale_camera_status) instance->registerEventListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_eventListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_data_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterDataListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IDepthDataListener> (handle, m_depthDataListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_depth_image_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterDepthImageListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IDepthImageListener> (handle, m_depthImageListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_ir_image_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterIRImageListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IIRImageListener> (handle, m_irImageListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_spc_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterSparsePointCloudListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::ISparsePointCloudListener> (handle, m_spcListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_event_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterEventListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IEventListener> (handle, m_eventListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_start_capture_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->startCapture();
}

ROYALE_CAPI royale_camera_status royale_camera_device_stop_capture_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->stopCapture();
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_max_sensor_width_v220 (royale_camera_handle handle, uint16_t *max_sensor_width)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->getMaxSensorWidth (*max_sensor_width);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_max_sensor_height_v220 (royale_camera_handle handle, uint16_t *max_sensor_height)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->getMaxSensorHeight (*max_sensor_height);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_lens_parameters_v210 (royale_camera_handle handle,
        royale_lens_parameters *params)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::LensParameters lParams;
    royale_camera_status status = (royale_camera_status) instance->getLensParameters (lParams);

    if (ROYALE_STATUS_SUCCESS != status)
    {
        return status;
    }

    params->principalPoint.first = lParams.principalPoint.first;
    params->principalPoint.second = lParams.principalPoint.second;

    params->focalLength.first = lParams.focalLength.first;
    params->focalLength.second = lParams.focalLength.second;

    params->distortionTangential.first = lParams.distortionTangential.first;
    params->distortionTangential.second = lParams.distortionTangential.second;

    params->distortionRadial.nrOfValues = 3;
    params->distortionRadial.values = (float *) malloc (sizeof (float) * 3);
    memcpy (params->distortionRadial.values, &lParams.distortionRadial[0], 3 * sizeof (float));

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_is_connected_v220 (royale_camera_handle handle, bool *is_connected)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->isConnected (*is_connected);
}

ROYALE_CAPI royale_camera_status royale_camera_device_is_calibrated_v220 (royale_camera_handle handle, bool *is_calibrated)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->isCalibrated (*is_calibrated);
}

ROYALE_CAPI royale_camera_status royale_camera_device_is_capturing_v220 (royale_camera_handle handle, bool *is_capturing)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->isCapturing (*is_capturing);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_access_level_v220 (royale_camera_handle handle, royale_camera_access_level *access_level)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    CameraAccessLevel level;
    royale_camera_status status = (royale_camera_status) instance->getAccessLevel (level);
    if (ROYALE_STATUS_SUCCESS == status)
    {
        *access_level = (royale_camera_access_level) level;
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_start_recording_v210 (royale_camera_handle handle, const char *filename,
        uint32_t nr_of_frames, uint32_t frame_skip, uint32_t ms_skip)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->startRecording (royale::String (filename), nr_of_frames, frame_skip, ms_skip);
}

ROYALE_CAPI royale_camera_status royale_camera_device_stop_recording_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->stopRecording();
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_record_stop_listener_v210 (royale_camera_handle handle,
        ROYALE_RECORD_STOP_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_record_stop_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<RecordStopListenerCAPI> (callback);
        royale_camera_status status = (royale_camera_status) instance->registerRecordListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_recordStopListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_exposure_listener_v210 (royale_camera_handle handle,
        ROYALE_EXPOSURE_CALLBACK_v210 callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_exposure_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<ExposureListenerCAPI> (callback);
        royale_camera_status status = (royale_camera_status) instance->registerExposureListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_exposureListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_exposure_listener_stream_v300 (royale_camera_handle handle,
        ROYALE_EXPOSURE_CALLBACK_v300 callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_exposure_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<ExposureListener2CAPI> (callback);
        royale_camera_status status = (royale_camera_status) instance->registerExposureListener (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_exposureListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_record_stop_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterRecordListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IRecordStopListener> (handle, m_recordStopListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_exposure_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterExposureListener());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IExposureListener2> (handle, m_exposureListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_frame_rate_v210 (royale_camera_handle handle, uint16_t frame_rate)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setFrameRate (frame_rate);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_frame_rate_v220 (royale_camera_handle handle, uint16_t *frame_rate)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->getFrameRate (*frame_rate);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_max_frame_rate_v220 (royale_camera_handle handle, uint16_t *max_frame_rate)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->getMaxFrameRate (*max_frame_rate);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_external_trigger_v330 (royale_camera_handle handle, bool use_external_trigger)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setExternalTrigger (use_external_trigger);
}

// ----------------------------------------------------------------------------------------------
// Level 2: Experienced users (Laser Class 1 guaranteed) - activation key required
// ----------------------------------------------------------------------------------------------

ROYALE_CAPI royale_camera_status royale_camera_device_get_exposure_groups_v300 (royale_camera_handle handle, char ***exposure_groups, uint32_t *nr_exposure_groups)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<String> royaleExposureGroups;
    royale_camera_status status = (royale_camera_status) instance->getExposureGroups (royaleExposureGroups);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *nr_exposure_groups = (uint32_t) royaleExposureGroups.size();

        // malloc even if there are no entries, to let the user be able to call free() without crashing the program
        *exposure_groups = (char **) malloc (sizeof (char *) * *nr_exposure_groups);

        for (uint32_t i = 0; i < *nr_exposure_groups; i++)
        {
            HelperFunctionsCAPI::copyRoyaleStringToCString (& (*exposure_groups) [i], royaleExposureGroups.at (i));
        }
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_times_v210 (royale_camera_handle handle,
        uint32_t *exposure_times, uint32_t nr_exposure_times)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::Vector<uint32_t> exTimes (exposure_times, exposure_times + nr_exposure_times);

    return (royale_camera_status) instance->setExposureTimes (exTimes);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_times_v300 (royale_camera_handle handle,
        royale_stream_id stream_id, uint32_t *exposure_times, uint32_t nr_exposure_times)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::Vector<uint32_t> exTimes (exposure_times, exposure_times + nr_exposure_times);

    return (royale_camera_status) instance->setExposureTimes (exTimes, stream_id);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_exposure_for_groups_v300 (royale_camera_handle handle,
        uint32_t *exposure_times, uint32_t nr_exposure_times)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::Vector<uint32_t> exTimes (exposure_times, exposure_times + nr_exposure_times);

    return (royale_camera_status) instance->setExposureForGroups (exTimes);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_group_exposure_time_v300 (royale_camera_handle handle, const char *exposure_group, uint32_t exposure_time)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setExposureTime (exposure_group, exposure_time);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_group_exposure_limits_v300 (royale_camera_handle handle, const char *exposure_group, uint32_t *lower_limit, uint32_t *upper_limit)
{
    *lower_limit = 0;
    *upper_limit = 0;

    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    Pair<uint32_t, uint32_t> exLimits;
    royale_camera_status status = (royale_camera_status) instance->getExposureLimits (exposure_group, exLimits);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *lower_limit = exLimits.first;
        *upper_limit = exLimits.second;
    }
    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_processing_parameters_v210 (royale_camera_handle handle,
        royale_processing_parameter **parameters, uint32_t nr_params)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::ProcessingParameterVector royaleParams;

    for (uint32_t i = 0; i < nr_params; i++)
    {
        royaleParams.push_back (ProcessingParametersConverterCAPI::toRoyaleProcessingParameter (& ( (*parameters) [i])));
    }

    return (royale_camera_status) instance->setProcessingParameters (royaleParams);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_processing_parameters_v300 (royale_camera_handle handle,
        royale_stream_id stream_id, royale_processing_parameter **parameters, uint32_t nr_params)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::ProcessingParameterVector royaleParams;

    for (uint32_t i = 0; i < nr_params; i++)
    {
        royaleParams.push_back (ProcessingParametersConverterCAPI::toRoyaleProcessingParameter (& ( (*parameters) [i])));
    }

    return (royale_camera_status) instance->setProcessingParameters (royaleParams, stream_id);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_processing_parameters_v210 (royale_camera_handle handle,
        royale_processing_parameter **parameters, uint32_t *nr_params)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::ProcessingParameterVector royaleParams;
    royale_camera_status status = (royale_camera_status) instance->getProcessingParameters (royaleParams);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *nr_params = (uint32_t) royaleParams.size();

        *parameters = (royale_processing_parameter *) malloc (sizeof (royale_processing_parameter) * (*nr_params));

        for (uint32_t i = 0; i < *nr_params; i++)
        {
            royale::Pair<ProcessingFlag, Variant> currentParameter = royaleParams.at (i);
            ProcessingParametersConverterCAPI::fromRoyaleProcessingParameter (& ( (*parameters) [i]), currentParameter);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_processing_parameters_v300 (royale_camera_handle handle,
        royale_stream_id stream_id, royale_processing_parameter **parameters, uint32_t *nr_params)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::ProcessingParameterVector royaleParams;
    royale_camera_status status = (royale_camera_status) instance->getProcessingParameters (royaleParams, stream_id);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *nr_params = (uint32_t) royaleParams.size();

        *parameters = (royale_processing_parameter *) malloc (sizeof (royale_processing_parameter) * (*nr_params));

        for (uint32_t i = 0; i < *nr_params; i++)
        {
            royale::Pair<ProcessingFlag, Variant> currentParameter = royaleParams.at (i);
            ProcessingParametersConverterCAPI::fromRoyaleProcessingParameter (& ( (*parameters) [i]), currentParameter);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_managed_set_proc_params (royale_camera_handle handle,
        royale_stream_id streamId, uint32_t *processingParameterArray, uint32_t totalLength)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);
    royale::ProcessingParameterVector processingParameterVector;
    for (uint32_t i = 0; i < totalLength; i += 3)
    {
        royale::ProcessingFlag flag = static_cast<royale::ProcessingFlag> (processingParameterArray[i]);
        royale::VariantType type = static_cast<royale::VariantType> (processingParameterArray[i + 1]);
        royale::Variant variant (type, processingParameterArray[i + 2]);

        royale::Pair < royale::ProcessingFlag, royale::Variant > pParamPair (flag, variant);

        processingParameterVector.push_back (pParamPair);
    }

    return (royale_camera_status) instance->setProcessingParameters (processingParameterVector, static_cast<royale::StreamId> (streamId));
}

ROYALE_CAPI royale_camera_status royale_camera_device_managed_get_proc_params (royale_camera_handle handle,
        royale_stream_id streamId, PROCESSING_PARAMETER_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    royale::ProcessingParameterVector parameterVector;
    royale::CameraStatus status = instance->getProcessingParameters (parameterVector, static_cast<royale::StreamId> (streamId));
    if (status == royale::CameraStatus::SUCCESS)
    {
        uint32_t totalLength = 0;

        totalLength = static_cast<uint32_t> (parameterVector.size()) * 3;

        uint32_t *processingParameterArray = (uint32_t *) ::malloc (sizeof (uint32_t) * (totalLength));

        int i_ppa = 0;

        for (size_t i = 0; i < parameterVector.size(); i++)
        {
            royale::Pair<royale::ProcessingFlag, royale::Variant> current = parameterVector.at (i);

            uint32_t flag = (uint32_t) current.first;
            uint32_t type = (uint32_t) current.second.variantType();
            uint32_t value = (uint32_t) current.second.getData();

            processingParameterArray[i_ppa++] = flag;
            processingParameterArray[i_ppa++] = type;
            processingParameterArray[i_ppa++] = value;
        }

        callback ( (uint32_t *) processingParameterArray, totalLength);
        free (processingParameterArray);
    }
    return (royale_camera_status) status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_register_extended_data_listener_v210 (royale_camera_handle handle,
        ROYALE_EXTENDED_DATA_CALLBACK callback)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = royale_camera_device_unregister_extended_data_listener (handle);
    if (status == ROYALE_STATUS_SUCCESS)
    {
        auto listener = common::makeUnique<ExtendedDataListenerCAPI> (callback);
        status = (royale_camera_status) instance->registerDataListenerExtended (listener.get());

        if (ROYALE_STATUS_SUCCESS == status)
        {
            m_extendedDataListenerMap[handle] = std::move (listener);
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_unregister_extended_data_listener_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    auto status = static_cast<royale_camera_status> (instance->unregisterDataListenerExtended());
    if (status == ROYALE_STATUS_SUCCESS)
    {
        delete_previously_created_listener<royale::IExtendedDataListener> (handle, m_extendedDataListenerMap);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_callback_data_v210 (royale_camera_handle handle, royale_callback_data cb_data)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setCallbackData ( (CallbackData) cb_data);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_callback_dataU16_v210 (royale_camera_handle handle, uint16_t cb_data)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setCallbackData (cb_data);
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_calibration_data_file (royale_camera_handle handle, char *file_name)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setCalibrationData (royale::String (file_name));
}

ROYALE_CAPI royale_camera_status royale_camera_device_set_calibration_data_v210 (royale_camera_handle handle, uint8_t **calibration_data,
        uint32_t nr_data_entries)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<uint8_t> calib_data (*calibration_data, *calibration_data + nr_data_entries);
    return (royale_camera_status) instance->setCalibrationData (calib_data);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_calibration_data_v210 (royale_camera_handle handle, uint8_t **calibration_data,
        uint32_t *nr_data_entries)
{
    *nr_data_entries = 0;
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<uint8_t> calib_data;

    royale_camera_status status = (royale_camera_status) instance->getCalibrationData (calib_data);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *nr_data_entries = (uint32_t) calib_data.size();

        *calibration_data = (uint8_t *) malloc (sizeof (uint8_t) * *nr_data_entries);
        memcpy (*calibration_data, &calib_data[0], *nr_data_entries);
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_managed_get_calib_data (royale_camera_handle handle,
        CALIBRATION_DATA_CALLBACK callback)
{
    uint8_t *calibration_data = nullptr;
    uint32_t nr_data_entries;

    royale_camera_status status = royale_camera_device_get_calibration_data_v210 (handle, &calibration_data, &nr_data_entries);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        callback (calibration_data, nr_data_entries);
    }

    free (calibration_data);

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_write_calibration_to_flash_v210 (royale_camera_handle handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->writeCalibrationToFlash();
}

// ----------------------------------------------------------------------------------------------
// Level 3: Advanced users (Laser Class 1 not (!) guaranteed) - activation key required
// ----------------------------------------------------------------------------------------------

ROYALE_CAPI royale_camera_status royale_camera_device_set_duty_cycle_v210 (royale_camera_handle handle, double duty_cycle,
        uint16_t index)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->setDutyCycle (duty_cycle, index);
}

ROYALE_CAPI royale_camera_status royale_camera_device_write_registers_v210 (royale_camera_handle handle,
        royale_pair_string_uint64 **registers, uint32_t nr_registers)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<Pair<String, uint64_t>> writeRegisters;

    for (uint32_t i = 0; i < nr_registers; i++)
    {
        royale_pair_string_uint64 currentRegister = (* (registers) [i]);
        writeRegisters.push_back (Pair <String, uint64_t> (currentRegister.first, currentRegister.second));
    }

    return (royale_camera_status) instance->writeRegisters (writeRegisters);
}

ROYALE_CAPI royale_camera_status royale_camera_device_read_registers_v210 (royale_camera_handle handle,
        royale_pair_string_uint64 **registers, uint32_t nr_registers)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<Pair<String, uint64_t>> readRegisters;

    for (uint32_t i = 0; i < nr_registers; i++)
    {
        royale_pair_string_uint64 currentRegister = * (registers) [i];
        readRegisters.push_back (Pair <String, uint64_t> (currentRegister.first, currentRegister.second));
    }

    royale_camera_status status = (royale_camera_status) instance->readRegisters (readRegisters);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        for (uint32_t i = 0; i < nr_registers; i++)
        {
            (* (registers) [i]).second = readRegisters[i].second;
        }
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_shift_lens_center_v320 (royale_camera_handle handle, int16_t tx, int16_t ty)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->shiftLensCenter (tx, ty);
}

ROYALE_CAPI royale_camera_status royale_camera_device_get_lens_center_v320 (royale_camera_handle handle, uint16_t *x, uint16_t *y)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    uint16_t cx, cy;
    royale_camera_status status = (royale_camera_status) instance->getLensCenter (cx, cy);

    if (ROYALE_STATUS_SUCCESS == status)
    {
        *x = cx;
        *y = cy;
    }

    return status;
}

ROYALE_CAPI royale_camera_status royale_camera_device_write_data_to_flash_v31000 (royale_camera_handle handle, uint8_t **flash_data,
        uint32_t nr_data_entries)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    Vector<uint8_t> data (*flash_data, *flash_data + nr_data_entries);
    return (royale_camera_status) instance->writeDataToFlash (data);
}

ROYALE_CAPI royale_camera_status royale_camera_device_write_data_to_flash_file_v31000 (royale_camera_handle handle, char *file_name)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->writeDataToFlash (royale::String (file_name));
}

// ----------------------------------------------------------------------------------------------
// Level 4: Direct imager access (Laser Class 1 not (!) guaranteed) - activation key required
// ----------------------------------------------------------------------------------------------

ROYALE_CAPI royale_camera_status royale_camera_device_initialize_with_use_case_v210 (royale_camera_handle handle,
        const char *init_use_case)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_STATUS_INVALID_HANDLE);

    return (royale_camera_status) instance->initialize (String (init_use_case));
}

ROYALE_CAPI_LINKAGE_BOTTOM
