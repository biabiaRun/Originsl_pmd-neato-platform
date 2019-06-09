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

#include <vector>
#include <cstdint>

namespace royale
{
    namespace pal
    {
        /**
         * The size of the I2C device's register addresses.
         */
        enum class I2cAddressMode
        {
            I2C_NO_ADDRESS,
            I2C_8BIT,
            I2C_16BIT
        };

        /**
         * An I2C bus, supporting arbitrary I2C access to the connected devices.
         *
         * Many sensors use II2cDeviceAccess instead of this interface, as encapsulates the I2C
         * address and makes fewer assumptions about the bus.  The convenience class
         * Access2I2cDeviceAdapter can wrap the II2cBusAccess to provide that interface.
         *
         * In v2.3.0 and the LTS branch: this class was called II2cAccess, and the sensors that now
         * use II2cDeviceAccess used IBridgeWithI2cSensors (or the BridgeWithI2cSensorsGeneric
         * wrapper).
         */
        class II2cBusAccess
        {
        public:
            virtual ~II2cBusAccess() = default;

            /**
             * Read register(s) based on a given slave address I2C address.  The size of the buffer
             * indicates the number of bytes to read.
             */
            virtual void readI2c (uint8_t devAddr,
                                  I2cAddressMode addrMode,
                                  uint16_t regAddr,
                                  std::vector<uint8_t> &buffer) = 0;

            /**
             * Write register(s) based on a given slave address I2C address.  The size of the buffer
             * indicates the number of bytes to write.
             */
            virtual void writeI2c (uint8_t devAddr,
                                   I2cAddressMode addrMode,
                                   uint16_t regAddr,
                                   const std::vector<uint8_t> &buffer) = 0;

            /**
             * Reconfigure the I2C bus's speed, which may mean that only some devices can be
             * communicated with.
             *
             * Callers should probably hold the lock from the low-level implementation (maybe
             * IUvcExtensionAccess::lockVendorExtension()) before calling this, and reset the
             * bus speed before releasing the lock.
             *
             * @param bps speed in bits per second
             */
            virtual void setBusSpeed (uint32_t bps) = 0;

            /**
             * The maximum size of the buffer for reading and writing.  This is a property of the
             * I2C bus and the buses / bridge used to implement this II2cBusAccess, it does not
             * include any limitation of the specific I2C device.
             */
            virtual std::size_t maximumDataSize () = 0;
        };
    }
}
