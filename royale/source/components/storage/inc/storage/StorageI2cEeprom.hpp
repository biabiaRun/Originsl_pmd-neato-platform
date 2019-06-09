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

#include <pal/II2cBusAccess.hpp>
#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <config/FlashMemoryConfig.hpp>

#include <chrono>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * Supports an EEPROM. If the address has the low bit clear, it's assumed to support 17-bit addressing.
         */
        class StorageI2cEeprom :
            public royale::pal::IStorageReadRandom,
            public royale::pal::IStorageWriteFullOverwrite
        {
        public:
            ROYALE_API StorageI2cEeprom (const royale::config::FlashMemoryConfig &config,
                                         std::shared_ptr<royale::pal::II2cBusAccess> access,
                                         uint8_t devAddr);
            ~StorageI2cEeprom () = default;

            // IStorageReadRandom
            void readStorage (std::size_t startAddr, std::vector<uint8_t> &buf) override;
            // IStorageWriteFullOverwrite
            void writeStorage (const std::vector<uint8_t> &buf) override;

        private:
            /**
             * The platform-specific low-level part of the control channel implementation.
             */
            std::shared_ptr<royale::pal::II2cBusAccess> m_access;

            /**
             * The addresses of the device.
             */
            uint8_t m_devAddr;

            /**
             * Implementation of readStorage, called with blocks small enough to read in a single
             * request.
             */
            void readStorageBlock (uint32_t startAddr, std::vector<uint8_t> &buf);

            /**
             * Page size for writing to the flash memory, all writes must be aligned to this, and a
             * maximum of this size.  Always non-zero.
             */
            std::size_t m_eepromWriteSize;

            /**
             * Delay between each write to the flash memory.
             */
            std::chrono::microseconds m_writeTime;

            /**
            * Size of the EEPROM
            */
            size_t m_imageSize;
        };
    }
}
