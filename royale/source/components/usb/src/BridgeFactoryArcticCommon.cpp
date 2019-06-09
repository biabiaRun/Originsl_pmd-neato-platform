/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/factory/BridgeFactoryArcticCommon.hpp>
#include <usb/bridge/BridgeImagerArctic.hpp>
#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/pal/DeviceControlArctic.hpp>
#include <usb/pal/GpioAccessArctic.hpp>
#include <usb/pal/I2cBusAccessArctic.hpp>
#include <usb/pal/SpiBusAccessArctic.hpp>

#include <common/RoyaleLogger.hpp>
#include <common/exceptions/NotImplemented.hpp>

using namespace royale::factory;
using namespace royale::usb::config;
using namespace royale::usb::bridge;

BridgeFactoryArcticCommon::BridgeFactoryArcticCommon() = default;

BridgeFactoryArcticCommon::~BridgeFactoryArcticCommon() = default;

void BridgeFactoryArcticCommon::initialize (std::shared_ptr<royale::hal::IBridgeDataReceiver> dataReceiver,
        std::shared_ptr<IUvcExtensionAccess> extensionAccess)
{
    m_bridge = dataReceiver;

    switch (extensionAccess->getVendorExtensionType())
    {
        case UvcExtensionType::None:
            LOG (WARN) << "Initialized UVC bridge without extension - this is probably not usable";
            break;

        case UvcExtensionType::Arctic:
            {
                using namespace royale::usb::pal::arctic;
                auto uvcExtension = std::make_shared<UvcExtensionArctic> (extensionAccess);
                m_bridgeImager = std::make_shared<BridgeImagerArctic> (uvcExtension);
                auto deviceControlArctic = std::make_shared<DeviceControlArctic> (uvcExtension);
                m_deviceControl = deviceControlArctic;
                m_gpioAccess = std::make_shared<GpioAccessArctic> (uvcExtension);
                m_i2cAccess = std::make_shared<I2cBusAccessArctic> (uvcExtension);
                m_spiAccess = std::make_shared<SpiBusAccessArctic> (uvcExtension);
                m_deviceStatus = deviceControlArctic;
            }
            break;

        default:
            LOG (ERROR) << "UVC bridge with unsupported extension - this is not usable";
            throw royale::common::NotImplemented ("UVC bridge with unsupported extension");
    }
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::hal::IBridgeImager> &b)
{
    b = m_bridgeImager;
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &b)
{
    b = m_bridge;
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::pal::IDeviceControl> &b)
{
    b = m_deviceControl;
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::pal::IGpioAccess> &b)
{
    b = m_gpioAccess;
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &b)
{
    b = m_i2cAccess;
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::pal::ISpiBusAccess> &b)
{
    b = m_spiAccess;
}

void BridgeFactoryArcticCommon::createImpl (std::shared_ptr<royale::usb::pal::IDeviceStatus> &b)
{
    b = m_deviceStatus;
}
