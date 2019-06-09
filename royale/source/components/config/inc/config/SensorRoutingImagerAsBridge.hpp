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

#include <common/ISensorRoutingConfig.hpp>

#include <royale/Definitions.hpp>

#include <cstdint>

namespace royale
{
    namespace config
    {
        /**
         * Enum for using the imager hardware for non-imaging tasks, for example as a peripheral
         * controller by using the imager's GPIO pins.
         *
         * Similar to ImagerType, this enum contains both the hardware and the firmware that is
         * running on that hardware.
         */
        enum class ImagerAsBridgeType
        {
            /**
             * Firmware which uses the M2452 A11 or M2452 B1x imager's GPIOs as an SPI bus master.
             */
            M2452_SPI,
            /**
             * The M2453, which doesn't need firmware to act as an SPI bus master.
             */
            M2453_SPI,
            /**
             * The M2455, which doesn't need firmware to act as an SPI bus master.
             */
            M2455_SPI,
        };

        /**
         * This SensorRoutingConfig indicates that the imager itself should be used as a peripheral
         * access chip, for example its GPIOs can act as an SPI bus master.
         *
         * Using this requires a different firmware than the normal image-capturing firmware, it
         * can only be used when the normal operation is not needed.  For example, during the module
         * probing the module is only accessible to the module probing code, and the SPI bus can be
         * used at that time.
         *
         * If used while a normal software imager is controlling the hardware imager, the behaviour
         * and eye-safety are undefined.
         */
        class SensorRoutingImagerAsBridge : public royale::common::ISensorRoutingConfig
        {
        public:
            explicit SensorRoutingImagerAsBridge (ImagerAsBridgeType imager) :
                m_imagerType {imager}
            {
            }

            ImagerAsBridgeType getImagerType() const
            {
                return m_imagerType;
            }

        private:
            const ImagerAsBridgeType m_imagerType;
        };
    }
}

