/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
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

#include <thread>

using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::usb::bridge::arctic;

BridgeImagerArctic::BridgeImagerArctic (std::shared_ptr<UvcExtensionArctic> extension) :
    m_extension {extension}
{
}

BridgeImagerArctic::~BridgeImagerArctic ()
{
}

void BridgeImagerArctic::setImagerReset (bool state)
{
    auto enumState = state ? ArcticGpioState::RESET_ENABLE : ArcticGpioState::RESET_DISABLE;
    m_extension->checkedSet (VendorRequest::GPIO_LOGICAL, enumState, LogicalGpio::RESET);
}
void BridgeImagerArctic::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    std::vector<uint8_t> result;
    result.resize (sizeof (uint16_t));

    // wValue is the I2C address, ignored for VendorRequest::I2C_IMAGER
    m_extension->checkedGet (VendorRequest::I2C_IMAGER, 0, regAddr, result);
    value = bufferToHostBe16 (result.data());
}
void BridgeImagerArctic::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    std::vector<uint8_t> data;
    pushBackBe16 (data, value);

    // wValue is the I2C address, ignored for VendorRequest::I2C_IMAGER
    m_extension->checkedSet (VendorRequest::I2C_IMAGER, 0, regAddr, data);
}

void BridgeImagerArctic::readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values)
{
    auto lock = m_extension->getUvcExtensionAccess()->lockVendorExtension();
    const std::size_t maxRegistersPerTransfer = MAXIMUM_DATA_SIZE / sizeof (uint16_t);
    const std::size_t lastBlock = values.size() - (values.size() % maxRegistersPerTransfer);

    // check that the register addresses won't wrap round
    (void) narrow_cast<uint16_t> (firstRegAddr + values.size());

    for (std::size_t i = 0; i < lastBlock; i += maxRegistersPerTransfer)
    {
        std::vector<uint8_t> data (maxRegistersPerTransfer * sizeof (uint16_t));
        m_extension->checkedGet (VendorRequest::I2C_IMAGER, 0, static_cast<uint16_t> (firstRegAddr + i), data);
        for (std::size_t j = 0; j < maxRegistersPerTransfer ; j++)
        {
            values[i + j] = bufferToHostBe16 (&data[sizeof (uint16_t) * j]);
        }
    }
    if (lastBlock != values.size())
    {
        const std::size_t lastBlockSize = values.size() - lastBlock;
        std::vector<uint8_t> data (lastBlockSize * sizeof (uint16_t));
        m_extension->checkedGet (VendorRequest::I2C_IMAGER, 0, static_cast<uint16_t> (firstRegAddr + lastBlock), data);
        for (std::size_t j = 0; j < lastBlockSize; j++)
        {
            values[lastBlock + j] = bufferToHostBe16 (&data[sizeof (uint16_t) * j]);
        }
    }
}

void BridgeImagerArctic::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    auto lock = m_extension->getUvcExtensionAccess()->lockVendorExtension();
    const std::size_t maxRegistersPerTransfer = MAXIMUM_DATA_SIZE / sizeof (uint16_t);
    const std::size_t lastBlock = values.size() - (values.size() % maxRegistersPerTransfer);

    // check that the register addresses won't wrap round
    (void) narrow_cast<uint16_t> (firstRegAddr + values.size());

    for (std::size_t i = 0; i < lastBlock; i += maxRegistersPerTransfer)
    {
        std::vector<uint8_t> data;
        for (std::size_t j = 0; j < maxRegistersPerTransfer ; j++)
        {
            pushBackBe16 (data, values[i + j]);
        }
        m_extension->checkedSet (VendorRequest::I2C_IMAGER, 0, static_cast<uint16_t> (firstRegAddr + i), data);
    }
    if (lastBlock != values.size())
    {
        std::vector<uint8_t> data;
        for (std::size_t i = lastBlock; i < values.size() ; i++)
        {
            pushBackBe16 (data, values[i]);
        }
        m_extension->checkedSet (VendorRequest::I2C_IMAGER, 0, static_cast<uint16_t> (firstRegAddr + lastBlock), data);
    }
}

void BridgeImagerArctic::sleepFor (std::chrono::microseconds sleepDuration)
{
    std::this_thread::sleep_for (sleepDuration);
}
