/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/factory/BridgeFactoryEnclustraLibUsb.hpp>
#include <usb/bridge/BridgeEnclustraLibUsb.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale::factory;
using namespace royale::usb::descriptor;
using namespace royale::config;
using namespace royale::usb::bridge;

BridgeFactoryEnclustraLibUsb::BridgeFactoryEnclustraLibUsb (std::unique_ptr<CameraDescriptorLibUsb> desc)
    : m_desc (std::move (desc)),
      m_bridge()
{
}

BridgeFactoryEnclustraLibUsb::~BridgeFactoryEnclustraLibUsb() = default;

void BridgeFactoryEnclustraLibUsb::initialize ()
{
    m_bridge.reset (new royale::usb::bridge::BridgeEnclustraLibUsb (std::move (m_desc)));
    m_bridge->openConnection();
}

void BridgeFactoryEnclustraLibUsb::createImpl (std::shared_ptr<royale::hal::IBridgeImager> &bridge)
{
    bridge = m_bridge;
}
void BridgeFactoryEnclustraLibUsb::createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &bridge)
{
    bridge = m_bridge;
}
void BridgeFactoryEnclustraLibUsb::createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &bridge)
{
    bridge = m_bridge;
}
void BridgeFactoryEnclustraLibUsb::createImpl (std::shared_ptr<royale::storage::IBridgeWithPagedFlash> &bridge)
{
    bridge = m_bridge;
}
