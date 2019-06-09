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

#include <usb/pal/SpiBusAccessArctic.hpp>
#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <usb/bridge/UvcExtensionArctic.hpp>
#include <config/FlashMemoryConfig.hpp>

#include <memory>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Supports a Serial Peripheral Interface flash storage attached to a CX3.
             *
             * The CX3 firmware has support specifically for storage devices, which this uses.
             * Each instance of this class represents one SPI slave within the target hardware.
             *
             * When used with FlashMemoryConfig.accessOffset, two instances of this class could
             * represent different areas of a single SPI device.
             *
             * This supports flash (needing a separate erase function).
             */
            class StorageSpiFlashArctic :
                public royale::pal::IStorageReadRandom,
                public royale::pal::IStorageWriteFullOverwrite
            {
            public:
                // The on-device size_t might not be the same as the host's.
                // \todo ROYAL-1314 change it to a uint32_t
                using device_spi_size_t = std::size_t;

                /**
                 * The deviceId argument is the logical identifier for the SPI SSN pin.
                 */
                ROYALE_API StorageSpiFlashArctic (const royale::config::FlashMemoryConfig &config,
                                       std::shared_ptr<royale::usb::pal::arctic::SpiBusAccessArctic> access,
                                       uint8_t deviceId);
                ~StorageSpiFlashArctic () = default;

                // IStorageReadRandom
                void readStorage (std::size_t startAddr, std::vector<uint8_t> &buf) override;
                // IStorageWriteFullOverwrite
                void writeStorage (const std::vector<uint8_t> &buf) override;

            private:
                /**
                 * Take ownership of the SPI SSN pins and exclusive use of the SPI bus.
                 *
                 * For Royale v1.7.0, it is mandatory to hold the exclusive ownership when calling
                 * any read/erase/write functions.  In future versions it may become optional.
                 */
                std::unique_lock<std::recursive_mutex> selectDevice();

                /**
                 * Write to the device, the sector being written to must already have been erased
                 * (or otherwise be in a ready state for this data).  The size of the buffer
                 * indicates the number of bytes to read.
                 */
                void writeSpi (device_spi_size_t startAddr, const std::vector<uint8_t> &buffer);

                /**
                 * Erase data - the sector size is handled in the firmware, but the caller must
                 * ensure that the startAddr is aligned on a sector boundary.
                 */
                void eraseSpiSector (device_spi_size_t startAddr);

                /**
                 * Returns true if the SPI master is currently erasing an SPI slave.  The caller
                 * should already be holding the lock from selectDevice.
                 */
                bool isWriteInProgress();

                /**
                 * The platform-specific low-level part of the control channel implementation.
                 */
                std::shared_ptr<royale::usb::pal::arctic::SpiBusAccessArctic> m_access;

                /**
                 * Logical address (SPI select ID) for CX3VendorExtension::SPI_SELECT_DEVICE.
                 */
                uint8_t m_devAddr;

                /**
                 * Implementation of readStorage, called with blocks small enough to read in a single
                 * request.
                 */
                void readStorageBlock (device_spi_size_t startAddr, std::vector<uint8_t> &buf);

                /**
                 * If non-zero, reads and writes are checked to ensure that they don't exceed the size
                 * of the memory.  If zero, the check is not done.
                 */
                device_spi_size_t m_imageSize;

                /**
                 * Page size for writing to the flash memory, all writes must be aligned to this, and a
                 * maximum of this size.  Always non-zero.
                 */
                device_spi_size_t m_eepromWriteSize;

                /**
                 * Size for erasing operations.  If zero, then reading is supporting but writing will
                 * throw an exception.
                 */
                device_spi_size_t m_sectorSize;

                /**
                 * The startAddr of readStorage, and the zero for writeStorage, are mapped to this
                 * offset in the underlying storage.
                 *
                 * The constructor will throw an exception if both m_sectorSize and m_accessOffset
                 * are non-zero, but m_accessOffset is not a multiple of the m_sectorSize.
                 */
                device_spi_size_t m_accessOffset;
            };
        }
    }
}
