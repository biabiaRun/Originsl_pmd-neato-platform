/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <CameraManagerCAPI.h>
#include <royale/CameraManager.hpp>
#include <private/CameraDeviceCreateCAPI.hpp>
#include <private/InstanceManagerCAPI.hpp>

using namespace royale;

InstanceManager<CameraManager *> m_instanceManager (::InstanceManagerType::CAMERA_MANAGER);

ROYALE_CAPI_LINKAGE_TOP

ROYALE_CAPI royale_cam_manager_hnd royale_camera_manager_create()
{
    return royale_camera_manager_create_with_code ("");
}

ROYALE_CAPI royale_cam_manager_hnd royale_camera_manager_create_with_code (const char *activation_code)
{
    auto manager = new CameraManager (royale::String (activation_code));
    if (manager == nullptr)
    {
        return ROYALE_NO_INSTANCE_CREATED;
    }

    royale_cam_manager_hnd h = m_instanceManager.AddInstance (manager);
    return h;
}

ROYALE_CAPI void royale_camera_manager_destroy (royale_cam_manager_hnd handle)
{
    auto manager = m_instanceManager.DeleteInstance (handle);
    if (manager != nullptr)
    {
        delete manager;
    }
}

ROYALE_CAPI royale_camera_access_level royale_camera_manager_get_access_level (royale_cam_manager_hnd handle, const char *activation_code)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_ACCESS_LEVEL1);

    return (royale_camera_access_level) instance->getAccessLevel (royale::String (activation_code));
}

ROYALE_CAPI char **royale_camera_manager_get_connected_cameras (royale_cam_manager_hnd handle, uint32_t *nr_cameras)
{
    return royale_camera_manager_get_connected_cameras_android (handle, nr_cameras, (uint32_t) (-1), (uint32_t) (-1), (uint32_t) (-1));
}

ROYALE_CAPI char **royale_camera_manager_get_connected_cameras_android (royale_cam_manager_hnd handle, uint32_t *nr_cameras, uint32_t android_usb_device_fd,
        uint32_t android_usb_device_vid, uint32_t android_usb_device_pid)
{
    *nr_cameras = 0;
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_NO_CAMERA_DEVICES_FOUND);

#if defined(TARGET_PLATFORM_ANDROID)
    auto cameras = instance->getConnectedCameraList (android_usb_device_fd, android_usb_device_vid, android_usb_device_pid);
#else
    auto cameras = instance->getConnectedCameraList();
#endif
    *nr_cameras = (uint32_t) cameras.size();

    char **camList = (char **) malloc (sizeof (char *) * *nr_cameras);

    for (uint32_t i = 0; i < *nr_cameras; i++)
    {

        royale::String str = cameras.at (i);
        uint32_t size = (uint32_t) str.length() + 1;
        camList[i] = (char *) malloc (sizeof (char) * size);
        memcpy ( (void *) camList[i], str.c_str(), size);
    }
    return camList;
}

ROYALE_CAPI royale_camera_handle royale_camera_manager_create_camera (royale_cam_manager_hnd handle, const char *camera_id)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_NO_INSTANCE_CREATED);

    auto camera = instance->createCamera (royale::String (camera_id));

    if (camera == nullptr)
    {
        return ROYALE_NO_INSTANCE_CREATED;
    }

    return royale_camera_device_create (std::shared_ptr<royale::ICameraDevice> (std::move (camera)));
}

ROYALE_CAPI royale_camera_handle royale_camera_manager_create_camera_with_trigger (royale_cam_manager_hnd handle, const char *camera_id, royale_trigger_mode trigger_mode)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (ROYALE_NO_INSTANCE_CREATED);

    auto triggerMode = static_cast<TriggerMode> (trigger_mode);
    auto camera = instance->createCamera (royale::String (camera_id), triggerMode);

    if (camera == nullptr)
    {
        return ROYALE_NO_INSTANCE_CREATED;
    }

    return royale_camera_device_create (std::shared_ptr<royale::ICameraDevice> (std::move (camera)));
}

ROYALE_CAPI_LINKAGE_BOTTOM
