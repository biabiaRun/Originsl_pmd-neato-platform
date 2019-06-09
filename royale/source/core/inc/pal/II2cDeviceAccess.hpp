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

#include <cstdint>
#include <vector>

namespace royale
{
    namespace pal
    {
        /**
         * Provides access to a single I2C slave address.
         *
         * This hides away the details of the connection (like which bus the device
         * is connected to and which slave address it uses).
         *
         * Note there are devices (e.g. large I2C EEPROMs) that have more than one slave address;
         * for handling these multiple II2cDeviceAccess instances (or the more generic II2cBusAccess
         * interface) would be needed.
         */
        class II2cDeviceAccess
        {
        public:
            virtual ~II2cDeviceAccess() = default;

            /**
             * Read from a device that doesn't need a register address to be specified.
             * Used for devices that only have one register, or support the notion of
             * "current" or "default" register.
             */
            virtual void readI2cNoAddress (std::vector<uint8_t> &buf) = 0;
            virtual void writeI2cNoAddress (const std::vector<uint8_t> &buf) = 0;
            /**
             * Read from a device that takes 8-bit register addresses.
             */
            virtual void readI2cAddress8 (uint8_t regAddr, std::vector<uint8_t> &buf) = 0;
            virtual void writeI2cAddress8 (uint8_t regAddr, const std::vector<uint8_t> &buf) = 0;
            /**
             * Read from a device that takes 16-bit register addresses.
             */
            virtual void readI2cAddress16 (uint16_t regAddr, std::vector<uint8_t> &buf) = 0;
            virtual void writeI2cAddress16 (uint16_t regAddr, const std::vector<uint8_t> &buf) = 0;
        };
    }
}
