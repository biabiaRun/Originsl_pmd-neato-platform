/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/factory/BridgeFactoryUvcV4l.hpp>
#include <usb/bridge/BridgeUvcV4l.hpp>
#include <usb/bridge/BridgeUvcV4lUvcExtension.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale::factory;
using namespace royale::usb::bridge;
using namespace royale::v4l::bridge;

BridgeFactoryUvcV4l::BridgeFactoryUvcV4l (const std::string &filename)
    : m_filename {filename}
{
}

BridgeFactoryUvcV4l::~BridgeFactoryUvcV4l() = default;

void BridgeFactoryUvcV4l::initialize ()
{
    LOG (INFO) << "Creating UVC V4l bridge";
    auto bridge = std::make_shared<BridgeUvcV4l> (m_filename);
    bridge->openConnection();
    auto extension = std::make_shared<BridgeUvcV4lUvcExtension> (bridge);
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
