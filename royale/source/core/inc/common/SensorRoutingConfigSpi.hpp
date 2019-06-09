/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
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

#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * How to communicate with an SPI device, given that the bridge can already (without the
         * information in this class) identify which SPI bus the device is directly connected to.
         *
         * This is a wrapper for a single logical address, which will be mapped to the SPI bus'
         * select pins (in a device-specific manner).
         */
        class SensorRoutingConfigSpi : public ISensorRoutingConfig
        {
        public:
            explicit SensorRoutingConfigSpi (uint8_t address)
                : m_address (address)
            {
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
