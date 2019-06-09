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

#include <storage/IBridgeWithPagedFlash.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <config/FlashMemoryConfig.hpp>

#include <royale/Definitions.hpp>

#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * The non-volatile storage that holds the serial number, calibration data, etc.
         *
         * This class handles StorageFormatType::PICO_LEGACY, also known as FlashMemoryPico and
         * FlashMemoryType::PICO_PAGED. It's used for pico flexx and older devices, but the newer
         * pico maxx and pico monstar do not use this format.
         *
         * This uses the IBridgeWithPagedFlash, a generic-sounding interface which requires the
         * StorageFormatPicoLegacy class to know about the page structure of the storage and is
         * currently only implemented by BridgeEnclustra. While the relevant IBridgeWithPagedFlash
         * could in principle be reimplemented for other bridge types, the IBridgeWithPagedFlash
         * interface (and the Pico Legacy format as a whole) is considered legacy, i.e. not
         * recommended for new designs.
         */
        class StorageFormatPicoLegacy : public royale::hal::INonVolatileStorage
        {
        public:
            ROYALE_API StorageFormatPicoLegacy (std::shared_ptr<IBridgeWithPagedFlash> bridge, const royale::config::FlashMemoryConfig &memoryConfig);
            ROYALE_API ~StorageFormatPicoLegacy() override = default;

            ROYALE_API royale::Vector<uint8_t> getModuleIdentifier() override;
            ROYALE_API royale::String getModuleSuffix() override;
            ROYALE_API royale::String getModuleSerialNumber() override;
            ROYALE_API royale::Vector<uint8_t> getCalibrationData() override;
            ROYALE_API uint32_t getCalibrationDataChecksum() override;

            ROYALE_API void writeCalibrationData (const royale::Vector<uint8_t> &data) override;

            /**
             * This will always throw, the Pico Legacy format does not store this extra data.
             *
             * In the identifier is supported (as read-only), but it's only meaningful in some
             * devices.
             */
            ROYALE_API void writeCalibrationData (const royale::Vector<uint8_t> &data,
                                                  const royale::Vector<uint8_t> &identifier,
                                                  const royale::String &suffix,
                                                  const royale::String &serialNumber) override;

        private:
            struct FlashHeader;
            /**
             * Reads the data via the Bridge, and parses it in to a FlashHeader.  The parsed
             * data returned is in native endianness and alignment. If the magic PMD entry was
             * not found, false is returned. This means no valid PMD data available.
             *
             * \param header output for where the data is stored
             * \return true if the magic PMD information was found and data is ok
             */
            bool readDataFromHardware (struct FlashHeader &header);

            std::shared_ptr<IBridgeWithPagedFlash> m_bridge;
            size_t m_imageSize;
            size_t m_pageSize;
            size_t m_sectorSize;
        };
    }
}
