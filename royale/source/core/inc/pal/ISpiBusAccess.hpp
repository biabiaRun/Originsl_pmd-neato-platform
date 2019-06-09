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

#include <cstdint>
#include <mutex>
#include <vector>

namespace royale
{
    namespace pal
    {
        /**
         * This is the generic interface for supporting arbitrary Serial Peripheral Interface
         * access to connected devices, each instance of this class represents one SPI master.
         *
         * This generic interface might not support the full features of the device.  For example,
         * the {@link royale::usb::bridge::StorageSpiFlashArctic} class uses firmware-specific
         * functions for EEPROM support.  Although it uses this class to control the SPI select pins
         * and the mutex for the bus, it also accesses the UvcExternsionArctic directly to do most
         * of the I/O.
         */
        class ISpiBusAccess
        {
        public:
            virtual ~ISpiBusAccess() = default;

            /**
             * Take ownership of the SPI SSN pins and exclusive use of the SPI bus.
             *
             * For Royale v1.7.0, it is mandatory to hold the exclusive ownership when calling
             * any read/erase/write functions.  In future versions it may become optional.
             *
             * @param id logical identifier for the SPI device to activate
             */
            virtual std::unique_lock<std::recursive_mutex> selectDevice (uint8_t id) = 0;

            /**
            * Reading from a SPI device using half-duplex mode.
            *
            * @param transmit a vector containing the data to send before reading from SPI
            * @param receive a vector with predefined size that will hold the received data
            */
            virtual void readSpi (const std::vector<uint8_t> &transmit, std::vector<uint8_t> &receive) = 0;

            /**
            * Transmit data to a SPI device
            *
            * @param transmit a vector containing the data to send to the SPI device
            */
            virtual void writeSpi (const std::vector<uint8_t> &transmit) = 0;

            /** Return value for maximumReadSize() */
            struct ReadSize
            {
                std::size_t transmit;
                std::size_t receive;
            };

            /**
             * The maximum size of the buffers when calling readSpi.  This is a property of device
             * acting as the master, it does not consider any limitation of the slave.
             *
             * If the maximum receive size depends on the transmit size, then the expected pair of
             * supported sizes will be returned. For example, a device that supports 260 bytes in
             * total is likely to return {4, 256}, even if it could also support {3, 257}.
             */
            virtual ReadSize maximumReadSize () = 0;

            /**
             * The maximum size of the buffer when calling writeSpi.  This is a property of
             * device acting as the master, it does not consider any limitation of the slave.
             */
            virtual std::size_t maximumWriteSize () = 0;
        };
    }
}
