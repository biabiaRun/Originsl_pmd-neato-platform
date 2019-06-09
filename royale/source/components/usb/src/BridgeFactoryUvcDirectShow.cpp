/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/factory/BridgeFactoryUvcDirectShow.hpp>
#include <usb/bridge/BridgeUvcDirectShow.hpp>
#include <usb/bridge/BridgeUvcWindowsUvcExtension.hpp>
#include <usb/pal/IDeviceStatus.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale::factory;
using namespace royale::usb::descriptor;
using namespace royale::usb::bridge;

BridgeFactoryUvcDirectShow::BridgeFactoryUvcDirectShow (std::unique_ptr<CameraDescriptorDirectShow> desc)
    : m_desc (std::move (desc))
{
}

BridgeFactoryUvcDirectShow::~BridgeFactoryUvcDirectShow() = default;

void BridgeFactoryUvcDirectShow::initialize ()
{
    auto bridge = std::make_shared<BridgeUvcDirectShow> (std::move (m_desc));
    bridge->openConnection();
    auto extension = std::make_shared<BridgeUvcWindowsUvcExtension> (bridge->getMediaSource());
    extension->openConnection();
    BridgeFactoryArcticCommon::initialize (bridge, extension);

    // Query the extension for info for the bridge
    auto deviceStatus = create<royale::usb::pal::IDeviceStatus> ();
    auto speed = deviceStatus->getUsbSpeed();
    bridge->setUsbSpeed (speed);
    auto transferFormat = deviceStatus->getUsbTransferFormat();
    bridge->setTransferFormat (transferFormat);
    auto firmwareVersion = deviceStatus->getFirmwareVersion();
    bridge->setUvcBridgeInfo (
    {
        { "UVC_FIRMWARE_VERSION", firmwareVersion },
        { "ARCTIC_FIRMWARE_VERSION", firmwareVersion },
        { "ARCTIC_FIRMWARE_TYPE", "UVC" }
    });
}
