/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <config/FlashMemoryConfig.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>
#include <hal/IBridgeImager.hpp>
#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>

#include <chrono>
#include <cstdint>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * With the special firmware in the ImagerSpiFirmwareM2452.cpp file, the M2452 imager can
         * act as an SPI bus master, and read and write the calibration storage.
         *
         * This class provides the Royale Platform Access Layer for accessing the storage device,
         * and can have StorageFormatPolar constructed on top of this layer.
         */
        class StorageSpiImagerM2452 : public royale::pal::IStorageReadRandom,
            public royale::pal::IStorageWriteFullOverwrite
        {
        public:
            ROYALE_API StorageSpiImagerM2452 (const royale::config::FlashMemoryConfig &config,
                                              std::shared_ptr<royale::hal::IBridgeImager> access,
                                              royale::config::ImagerAsBridgeType firmwareType);

            /**
             * This destructor will reset the imager, to ensure it does not continue to run
             * SPIMaster firmware when accessed by code expecting a normal imager.
             */
            ~StorageSpiImagerM2452() override;

            void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) override;
            void writeStorage (const std::vector<uint8_t> &buffer) override;

        private:
            void resetAndLoadFirmware();

        private:
            /**
             * If the firmware isn't already loaded, load it.
             */
            void loadFirmwareOnce();

            /**
             * True once loadFirmwareOnce has been called.
             */
            bool m_firmwareLoaded = false;

            std::shared_ptr<royale::hal::IBridgeImager> m_access;
            const royale::config::ImagerAsBridgeType m_firmwareType;

            /**
             * This will unconditionally sleep for firstSleep, and then in a loop read register reg,
             * if it's non-zero it will sleep for pollSleep and then retry (repeatedly).
             *
             * \throw Timeout after a number of retries (the number is defined in the function)
             */
            void pollUntilZero (uint16_t reg, const std::chrono::microseconds firstSleep, const std::chrono::microseconds pollSleep);

            /**
             * Delay between each erase and beginning to poll to see if it is completed.
             */
            std::chrono::microseconds m_eraseTime;

            /**
             * Delay between each read and beginning to poll to see if it is completed.
             */
            std::chrono::microseconds m_readTime;

            /**
             * Delay between each write and beginning to poll to see if it is completed.
             */
            std::chrono::microseconds m_writeTime;

            /**
             * Size of the storage area
             */
            std::size_t m_imageSize;
        };
    }
}
