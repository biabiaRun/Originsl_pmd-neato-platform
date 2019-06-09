/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <common/ISensorRoutingConfig.hpp>

#include <royale/Definitions.hpp>
#include <common/exceptions/InvalidValue.hpp>

#include <cstdint>

namespace royale
{
    namespace common
    {

        /**
         * How to communicate with an I2C sensor, given that the bridge can already (without the information in this class) identify which I2C bridge the sensor is directly connected to.
         *
         * This is a wrapper for a single 7-bit I2C address.
         */
        class SensorRoutingConfigI2c : public ISensorRoutingConfig
        {
        public:
            explicit SensorRoutingConfigI2c (uint8_t address)
                : m_address (address)
            {
                if ( (address < 0x08) || (address >= 0x78))
                {
                    throw InvalidValue ("Invalid or reserved I2C address");
                }
            }

            uint8_t getAddress() const
            {
                return m_address;
            }

        private:
            const uint8_t m_address;
        };
    }
}
