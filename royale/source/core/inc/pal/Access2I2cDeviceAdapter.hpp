/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <pal/II2cBusAccess.hpp>
#include <pal/II2cDeviceAccess.hpp>
#include <common/SensorRoutingConfigI2c.hpp>

#include <memory>

namespace royale
{
    namespace pal
    {
        /**
         * II2cDeviceAccess adapter for II2cBusAccess.
         *
         * Provides access to a single I2C slave.
         */
        class Access2I2cDeviceAdapter : public II2cDeviceAccess
        {
        public:
            Access2I2cDeviceAdapter (std::shared_ptr<II2cBusAccess> access,
                                     uint8_t slaveAddr);
            Access2I2cDeviceAdapter (std::shared_ptr<II2cBusAccess> access,
                                     const common::SensorRoutingConfigI2c &sensorRouting);
            ~Access2I2cDeviceAdapter();

            // implement II2cDeviceAccess
            void readI2cNoAddress (std::vector<uint8_t> &buf) override;
            void writeI2cNoAddress (const std::vector<uint8_t> &buf) override;
            void readI2cAddress8 (uint8_t regAddr, std::vector<uint8_t> &buf) override;
            void writeI2cAddress8 (uint8_t regAddr, const std::vector<uint8_t> &buf) override;
            void readI2cAddress16 (uint16_t regAddr, std::vector<uint8_t> &buf) override;
            void writeI2cAddress16 (uint16_t regAddr, const std::vector<uint8_t> &buf) override;

        private:
            std::shared_ptr<II2cBusAccess> m_access;
            uint8_t                        m_slaveAddr;
        };
    }
}
