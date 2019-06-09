/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

// This file uses the CyAPI library from Cypress which is licensed under the Cypress
// Software License Agreement (see cypress_license.txt)

#include <usb/factory/BridgeFactoryEnclustraCyApi.hpp>
#include <usb/bridge/BridgeEnclustraCyApi.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/RoyaleLogger.hpp>

#include <guiddef.h>
#include <initguid.h>
#include <Windows.h>
#include <CyAPI.h>

using namespace royale::factory;

BridgeFactoryEnclustraCyApi::BridgeFactoryEnclustraCyApi (std::unique_ptr<CCyUSBDevice> device)
    : m_device (std::move (device)),
      m_bridge()
{
}

BridgeFactoryEnclustraCyApi::~BridgeFactoryEnclustraCyApi() = default;

void BridgeFactoryEnclustraCyApi::initialize ()
{
    if (m_device == nullptr)
    {
        throw common::LogicError ("Factory: trying to initialize two bridges from one device");
    }

    m_bridge.reset (new royale::usb::bridge::BridgeEnclustraCyApi (std::move (m_device)));
    m_bridge->openConnection();
}



void BridgeFactoryEnclustraCyApi::createImpl (std::shared_ptr<royale::hal::IBridgeImager> &bridge)
{
    bridge = m_bridge;
}
void BridgeFactoryEnclustraCyApi::createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &bridge)
{
    bridge = m_bridge;
}
void BridgeFactoryEnclustraCyApi::createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &bridge)
{
    bridge = m_bridge;
}
void BridgeFactoryEnclustraCyApi::createImpl (std::shared_ptr<royale::storage::IBridgeWithPagedFlash> &bridge)
{
    bridge = m_bridge;
}
