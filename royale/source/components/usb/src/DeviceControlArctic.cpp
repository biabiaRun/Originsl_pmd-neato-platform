/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/ArcticProtocolConstants.hpp>
#include <usb/pal/DeviceControlArctic.hpp>

#include <common/exceptions/Disconnected.hpp>

using namespace royale::buffer;
using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::usb::bridge::arctic;
using namespace royale::usb::pal;

DeviceControlArctic::DeviceControlArctic (std::shared_ptr<UvcExtensionArctic> extension) :
    m_extension {extension}
{
}

DeviceControlArctic::~DeviceControlArctic ()
{
}

void DeviceControlArctic::resetDeviceCommon (DeviceResetType resetType)
{
    auto lock = m_extension->getUvcExtensionAccess()->lockVendorExtension();
    m_extension->uncheckedSet (VendorRequest::DEVICE_RESET, resetType, 0);
    try
    {
        m_extension->checkError (false);
    }
    catch (const Disconnected &)
    {
        // Device successfully reset.  Depending on the firmware, a soft reset might also have
        // succeeded even if it didn't cause a Disconnected exception.
    }
    // all other exceptions from checkError are thrown to the caller
}

void DeviceControlArctic::resetDevice()
{
    resetDeviceCommon (DeviceResetType::ANY_SUPPORTED);
}

void DeviceControlArctic::resetDeviceHard()
{
    resetDeviceCommon (DeviceResetType::API_HARD_RESET);
}

UsbSpeed DeviceControlArctic::getUsbSpeed()
{
    auto speed = m_extension->getSpeed();
    switch (speed)
    {
        case ArcticUsbSpeed::FULL:
            return UsbSpeed::FULL;
        case ArcticUsbSpeed::HIGH:
            return UsbSpeed::HIGH;
        case ArcticUsbSpeed::SUPER:
            return UsbSpeed::SUPER;
        default:
        case ArcticUsbSpeed::UNKNOWN:
            return UsbSpeed::UNKNOWN;
    }
}

BufferDataFormat DeviceControlArctic::getUsbTransferFormat()
{
    auto format = m_extension->getTransferFormat();
    switch (format)
    {
        case ArcticUsbTransferFormat::RAW12:
            return BufferDataFormat::RAW12;
        case ArcticUsbTransferFormat::RAW16:
            return BufferDataFormat::RAW16;
        default:
            return BufferDataFormat::UNKNOWN;
    }
}

void DeviceControlArctic::setSpiDisableAndHighZ (bool disable)
{
    auto lock = m_extension->getUvcExtensionAccess()->lockVendorExtension();
    const auto wValue = disable ? DisableAndHighZ::SPI_DISABLE_AND_HIGH_Z : DisableAndHighZ::SPI_ENABLE;
    m_extension->checkedSet (VendorRequest::PERIPHERAL_DISABLE_AND_HIGH_Z, wValue, 0);
}

royale::String DeviceControlArctic::getFirmwareVersion()
{
    auto version = m_extension->getFirmwareVersion();
    return royale::String ("arctic_v") + royale::String::fromUInt (version.major)
           + "." + royale::String::fromUInt (version.minor)
           + "." + royale::String::fromUInt (version.patch);
}
