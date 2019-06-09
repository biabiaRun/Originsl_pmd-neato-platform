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
#include <usb/bridge/BridgeImagerArctic.hpp>

#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <common/exceptions/InvalidValue.hpp>

#include <usb/pal/I2cBusAccessArctic.hpp>

using namespace royale::usb::bridge;
using namespace royale::usb::bridge::arctic;
using namespace royale::usb::pal::arctic;
using namespace royale::pal;

using namespace royale::common;

I2cBusAccessArctic::I2cBusAccessArctic (std::shared_ptr<UvcExtensionArctic> extension) :
    m_extension {std::move (extension) }
{
}

void I2cBusAccessArctic::readI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, std::vector<uint8_t> &buffer)
{
    VendorRequest request;
    switch (addrMode)
    {
        case I2cAddressMode::I2C_NO_ADDRESS:
            request = VendorRequest::I2C_NO_ADDRESS;
            regAddr = 0;
            break;
        case I2cAddressMode::I2C_8BIT:
            request = VendorRequest::I2C_8_ADDRESS;
            (void) narrow_cast<uint8_t> (regAddr);
            break;
        case I2cAddressMode::I2C_16BIT:
            request = VendorRequest::I2C_16_ADDRESS;
            break;
        default:
            throw LogicError ();
    }
    m_extension->checkedGet (request, devAddr, regAddr, buffer);
}

void I2cBusAccessArctic::writeI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, const std::vector<uint8_t> &buffer)
{
    VendorRequest request;
    switch (addrMode)
    {
        case I2cAddressMode::I2C_NO_ADDRESS:
            request = VendorRequest::I2C_NO_ADDRESS;
            regAddr = 0;
            break;
        case I2cAddressMode::I2C_8BIT:
            request = VendorRequest::I2C_8_ADDRESS;
            (void) narrow_cast<uint8_t> (regAddr);
            break;
        case I2cAddressMode::I2C_16BIT:
            request = VendorRequest::I2C_16_ADDRESS;
            break;
        default:
            throw LogicError ();
    }
    m_extension->checkedSet (request, devAddr, regAddr, buffer);
}

void I2cBusAccessArctic::setBusSpeed (uint32_t bps)
{
    m_extension->checkedSet (VendorRequest::I2C_BUS_SPEED, static_cast<uint16_t> (bps >> 16), static_cast<uint16_t> (bps & 0xffff));
}

std::size_t I2cBusAccessArctic::maximumDataSize()
{
    return royale::usb::bridge::arctic::MAXIMUM_DATA_SIZE;
}
