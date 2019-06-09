/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

// This file uses the CyAPI library from Cypress which is licensed under the Cypress
// Software License Agreement (see cypress_license.txt)

#include <usb/factory/BridgeFactoryAmundsenCyApi.hpp>
#include <usb/bridge/BridgeAmundsenCyApi.hpp>
#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/pal/IDeviceStatus.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale::factory;
using namespace royale::usb::bridge;

BridgeFactoryAmundsenCyApi::BridgeFactoryAmundsenCyApi (std::unique_ptr<CCyUSBDevice> device)
    : m_device (std::move (device))
{
}

BridgeFactoryAmundsenCyApi::~BridgeFactoryAmundsenCyApi() = default;

void BridgeFactoryAmundsenCyApi::initialize ()
{
    auto bridge = std::make_shared<BridgeAmundsenCyApi> (std::move (m_device));
    bridge->openConnection();
    BridgeFactoryArcticCommon::initialize (bridge, bridge);

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
        { "ARCTIC_FIRMWARE_TYPE", "Amundsen" }
    });
}
