/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/factory/BridgeFactoryAmundsenLibUsb.hpp>
#include <usb/bridge/BridgeAmundsenLibUsb.hpp>
#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/pal/IDeviceStatus.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale::factory;
using namespace royale::usb::bridge;

namespace
{
    royale::String infoForDeviceType (BridgeAmundsenLibUsbDeviceType deviceType)
    {
        switch (deviceType)
        {
            case BridgeAmundsenLibUsbDeviceType::AMUNDSEN:
                return "Amundsen";
            case BridgeAmundsenLibUsbDeviceType::UVC:
                return "UVC";
            default:
                // should not happen, but allow whatever is parsing the bridge info to decide what
                // to do about this
                return "UNKNOWN";
        }
    }
}

BridgeFactoryAmundsenLibUsb::BridgeFactoryAmundsenLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> desc)
    : m_desc (std::move (desc))
{
}

BridgeFactoryAmundsenLibUsb::~BridgeFactoryAmundsenLibUsb() = default;

void BridgeFactoryAmundsenLibUsb::initialize ()
{
    auto bridge = std::make_shared<BridgeAmundsenLibUsb> (std::move (m_desc));
    bridge->openConnection();
    BridgeFactoryArcticCommon::initialize (bridge, bridge);

    // Query the extension for info for the bridge
    auto deviceStatus = create<royale::usb::pal::IDeviceStatus> ();
    auto speed = deviceStatus->getUsbSpeed();
    bridge->setUsbSpeed (speed);
    auto transferFormat = deviceStatus->getUsbTransferFormat();
    bridge->setTransferFormat (transferFormat);
    auto firmwareVersion = deviceStatus->getFirmwareVersion();

    // Query the device type and add it to the bridge info, so that the bridge doesn't need to know
    // anything about the contents of the bridge info.
    auto deviceType = bridge->getDeviceType();

    bridge->setUvcBridgeInfo (
    {
        { "UVC_FIRMWARE_VERSION", firmwareVersion },
        { "ARCTIC_FIRMWARE_VERSION", firmwareVersion },
        { "ARCTIC_FIRMWARE_TYPE", infoForDeviceType (deviceType) }
    });
}
