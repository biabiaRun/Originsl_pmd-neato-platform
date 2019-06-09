/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <pal/Access2I2cDeviceAdapter.hpp>

using namespace royale::pal;

Access2I2cDeviceAdapter::Access2I2cDeviceAdapter (std::shared_ptr<II2cBusAccess> access,
        uint8_t slaveAddr)
    : m_access (access),
      m_slaveAddr (slaveAddr)
{
}

Access2I2cDeviceAdapter::Access2I2cDeviceAdapter (std::shared_ptr<II2cBusAccess> access,
        const royale::common::SensorRoutingConfigI2c &sensorRouting)
    : m_access (access),
      m_slaveAddr (sensorRouting.getAddress())
{
}

Access2I2cDeviceAdapter::~Access2I2cDeviceAdapter () = default;

void Access2I2cDeviceAdapter::readI2cNoAddress (std::vector<uint8_t> &recvBuffer)
{
    auto regAddr = uint8_t {0}; // ignored or NO_ADDRESS
    m_access->readI2c (m_slaveAddr, I2cAddressMode::I2C_NO_ADDRESS, regAddr, recvBuffer);
}

void Access2I2cDeviceAdapter::writeI2cNoAddress (const std::vector<uint8_t> &buf)
{
    auto regAddr = uint8_t {0}; // ignored or NO_ADDRESS
    m_access->writeI2c (m_slaveAddr, I2cAddressMode::I2C_NO_ADDRESS, regAddr, buf);
}

void Access2I2cDeviceAdapter::readI2cAddress8 (uint8_t  regAddr, std::vector<uint8_t> &recvBuffer)
{
    m_access->readI2c (m_slaveAddr, I2cAddressMode::I2C_8BIT, regAddr, recvBuffer);
}

void Access2I2cDeviceAdapter::writeI2cAddress8 (uint8_t  regAddr, const std::vector<uint8_t> &buf)
{
    m_access->writeI2c (m_slaveAddr, I2cAddressMode::I2C_8BIT, regAddr, buf);
}

void Access2I2cDeviceAdapter::readI2cAddress16 (uint16_t regAddr, std::vector<uint8_t> &recvBuffer)
{
    m_access->readI2c (m_slaveAddr, I2cAddressMode::I2C_16BIT, regAddr, recvBuffer);
}

void Access2I2cDeviceAdapter::writeI2cAddress16 (uint16_t regAddr, const std::vector<uint8_t> &buf)
{
    m_access->writeI2c (m_slaveAddr, I2cAddressMode::I2C_16BIT, regAddr, buf);
}
