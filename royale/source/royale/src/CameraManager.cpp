/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventImagerConfigNotFound.hpp>
#include <common/exceptions/APIExceptionHandling.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/ImagerConfigNotFoundError.hpp>
#include <common/MakeUnique.hpp>
#include <config/ICoreConfig.hpp>
#include <device/CameraCore.hpp>
#include <device/CameraDevice.hpp>
#include <factory/BridgeController.hpp>
#include <record/CameraPlayback.hpp>
#include <royale/CameraManager.hpp>
#include <DeviceFactory.hpp>
#include <FileSystem.hpp>
#include <RoyaleLogger.hpp>

#include <cassert>
#include <map>

#ifdef ROYALE_VERSION_DIRTY
#include <common/DirtyVersion.hpp>
#endif

using namespace royale;

struct CameraManager::CameraManagerData
{
    CameraManagerData() :
        accessLevel (CameraAccessLevel::L1),
        eventQueue (std::make_shared<royale::EventQueue> ())
    {
    }

    royale::Vector<Pair<royale::String, std::unique_ptr<ICameraDevice>>> cameraDevices;
    CameraAccessLevel accessLevel;
    const std::shared_ptr<royale::EventQueue> eventQueue;
};

CameraManager::CameraManager (const royale::String &activationCode) :
    m_data (new CameraManagerData())
{
    m_data->accessLevel = getAccessLevel (activationCode);
}

CameraAccessLevel CameraManager::getAccessLevel (const royale::String &activationCode)
{
    if (activationCode == ROYALE_ACCESS_CODE_LEVEL2)
    {
        return CameraAccessLevel::L2;
    }
    else if (activationCode == ROYALE_ACCESS_CODE_LEVEL3)
    {
        return CameraAccessLevel::L3;
    }
    else if (activationCode == ROYALE_ACCESS_CODE_LEVEL4)
    {
        return CameraAccessLevel::L4;
    }

    return CameraAccessLevel::L1;
}

CameraManager::~CameraManager()
{
    // close connections of cameras which are not used
    for (size_t i = 0; i < m_data->cameraDevices.size(); ++i)
    {
        if (m_data->cameraDevices.at (i).second != nullptr)
        {
            m_data->cameraDevices.at (i).second = nullptr;
        }
    }

    unregisterEventListener ();
}
#if defined(TARGET_PLATFORM_ANDROID)
royale::Vector<royale::String> CameraManager::getConnectedCameraList (uint32_t androidUsbDeviceFD,
        uint32_t androidUsbDeviceVid,
        uint32_t androidUsbDevicePid)
#else
royale::Vector<royale::String> CameraManager::getConnectedCameraList()
#endif
{
    royale::Vector<royale::String> cameraIds;
    m_cameraNames.clear();

    // empty detected camera list
    m_data->cameraDevices.clear();

    royale::factory::BridgeController bridgeController;

    bridgeController.setEventListener (m_data->eventQueue);
    // only devices which are not yet consumed by createCamera() will pop-up here
#if defined(TARGET_PLATFORM_ANDROID)
    auto cameraPartsList = bridgeController.probeDevices (androidUsbDeviceFD,
                           androidUsbDeviceVid,
                           androidUsbDevicePid);
#else
    auto cameraPartsList = bridgeController.probeDevices();
#endif

    if (cameraPartsList.empty())
    {
        return cameraIds;
    }

    for (std::size_t i = 0; i < cameraPartsList.size(); i++)
    {
        try
        {
            LOG (INFO) << "Trying to open device #" << i << " of " << cameraPartsList.size();

            std::unique_ptr<ICameraDevice> cameraDevice (DeviceFactory::createCameraDevice (m_data->accessLevel, *cameraPartsList[i]));

            assert (cameraDevice != nullptr);

            royale::String serialNumber;
            auto ret = cameraDevice->getId (serialNumber);
            if (ret != CameraStatus::SUCCESS)
            {
                LOG (WARN) << "CameraManager::getConnectedCameraList: connected but could not read the ID - "
                           << getStatusString (ret);
                continue;
            }
            cameraIds.push_back (serialNumber);

            royale::String cameraName;
            ret = cameraDevice->getCameraName (cameraName);
            if (ret != CameraStatus::SUCCESS)
            {
                LOG (WARN) << "CameraManager::getConnectedCameraList: connected but could not read the Name - "
                           << getStatusString (ret);
                continue;
            }
            m_cameraNames.push_back (cameraName);

            Pair<royale::String, std::unique_ptr<ICameraDevice>> pair (serialNumber, std::move (cameraDevice));
            m_data->cameraDevices.push_back (std::move (pair));
        }
        catch (royale::common::ImagerConfigNotFoundError &e)
        {
            auto event = common::makeUnique<event::EventImagerConfigNotFound>();
            const auto &cameraCoreBuilder = cameraPartsList[i];
            const auto &coreConfig = cameraCoreBuilder->getICoreConfig();
            const String cameraName = coreConfig->getCameraName();

            event->setCameraName (cameraName);
            event->setConfigFileName (e.getConfigFileName());
            m_data->eventQueue->onEvent (std::move (event));
        }
        catch (royale::common::Exception &e)
        {
            LOG (WARN)
                    << "CameraManager::getConnectedCameraList: cannot create a camera module (maybe it is already in use) - "
                    << getStatusString (e.getStatus())
                    << ": "
                    << e.getTechnicalDescription();
        }
    }

    return cameraIds;
}

std::unique_ptr<ICameraDevice> CameraManager::createCamera (const royale::String &cameraId, const royale::TriggerMode mode)
{
    // if ID is not found, return check if it is a file name pointing to a replay file
    for (size_t i = 0; i < m_data->cameraDevices.size(); ++i)
    {
        if (m_data->cameraDevices.at (i).first == cameraId)
        {
#ifdef ROYALE_VERSION_DIRTY
            royale::common::showDirtyVersionWarning();
#endif
            if (mode == TriggerMode::SLAVE)
            {
                // If the camera is created as a slave we have to tell it
                auto ret = m_data->cameraDevices[i].second->setExternalTrigger (true);
                if (ret != CameraStatus::SUCCESS)
                {
                    return nullptr;
                }
            }

            // \todo we should delete the entry after returning !!!
            return std::move (m_data->cameraDevices[i].second);
        }
    }

    if (royale::common::fileexists (String::toStdString (cameraId)))
    {
        std::unique_ptr<ICameraDevice> cameraDevice (new record::CameraPlayback (
                    m_data->accessLevel,
                    String::toStdString (cameraId)));
#ifdef ROYALE_VERSION_DIRTY
        royale::common::showDirtyVersionWarning ();
#endif
        return cameraDevice;
    }
    else
    {
        return nullptr;
    }
}

royale::Vector<royale::String> CameraManager::getConnectedCameraNames()
{
    return m_cameraNames;
}

ROYALE_API royale::CameraStatus CameraManager::registerEventListener (
    royale::IEventListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    CameraStatus status = CameraStatus::SUCCESS;

    try
    {
        m_data->eventQueue->setEventListener (listener);
    }
    catch (royale::common::Exception &e)
    {
        status = e.getStatus();
    }

    return status;
}
ROYALE_API_EXCEPTION_SAFE_END

ROYALE_API royale::CameraStatus CameraManager::unregisterEventListener ()
ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    CameraStatus status = CameraStatus::SUCCESS;

    try
    {
        m_data->eventQueue->sync();
        m_data->eventQueue->setEventListener (nullptr);
    }
    catch (royale::common::Exception &e)
    {
        status = e.getStatus();
    }

    return status;
}
ROYALE_API_EXCEPTION_SAFE_END
