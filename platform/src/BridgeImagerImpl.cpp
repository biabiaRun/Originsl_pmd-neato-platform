/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "BridgeImagerImpl.hpp"

#include <arpa/inet.h>

#include <imager/M2450_A12/ImagerRegisters.hpp>

#include <thread>

namespace platform
{

BridgeImagerImpl::BridgeImagerImpl(
        std::shared_ptr<royale::pal::Access2I2cDeviceAdapter> i2c_adapter) :
    m_adapter(i2c_adapter)

{
}

BridgeImagerImpl::~BridgeImagerImpl()
{

}

void BridgeImagerImpl::setImagerReset (bool state)
{
}

void BridgeImagerImpl::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    std::vector<uint8_t> buf(sizeof(uint16_t));
    m_adapter->readI2cAddress16(regAddr, buf);
    value = ntohs(*reinterpret_cast<uint16_t*>(buf.data()));
}

void BridgeImagerImpl::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    std::vector<uint8_t> buf(sizeof(uint16_t));
    *reinterpret_cast<uint16_t*>(buf.data()) = htons(value);
    m_adapter->writeI2cAddress16(regAddr, buf);
}

void BridgeImagerImpl::readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values)
{
    std::vector<uint8_t> buf(values.size() * sizeof(uint16_t));
    m_adapter->readI2cAddress16(firstRegAddr, buf);
    uint16_t *cur = reinterpret_cast<uint16_t*>(buf.data());
    for(uint16_t& val : values)
        val = ntohs(*cur++);
}

void BridgeImagerImpl::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    std::vector<uint8_t> buf(values.size() * sizeof(uint16_t));
    uint16_t *cur = reinterpret_cast<uint16_t*>(buf.data());
    for(const uint16_t& val : values)
        *cur++ = htons(val);
    m_adapter->writeI2cAddress16(firstRegAddr, buf);
}

void BridgeImagerImpl::sleepFor (std::chrono::microseconds sleepDuration)
{
    std::this_thread::sleep_for (sleepDuration);
}

} // namespace platform
