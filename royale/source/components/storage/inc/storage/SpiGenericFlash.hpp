/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <common/SensorRoutingConfigSpi.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>
#include <hal/IBridgeImager.hpp>
#include <pal/ISpiBusAccess.hpp>
#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteSectorBased.hpp>

#include <chrono>
#include <cstdint>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * Support for WS25Q10EW, M25P40, or compatible storage devices, which are used in the
         * Infineon and PMD reference designs for M2453 and M2455 with SPI flash attached directly
         * to the imager.
         *
         * This class provides the Royale Platform Access Layer for accessing the storage device,
         * and can have StorageFormatPolar or StorageFormatZwetschge constructed on top of this
         * layer.
         */
        class SpiGenericFlash : public royale::pal::IStorageReadRandom,
            public royale::pal::IStorageWriteSectorBased
        {
        public:
            /**
             * For the SPI memory accessed via the imager, size_t would be 24-bit. This type is
             * defined just to indicate which device's size_t the code is referring to.
             */
            using device_spi_size_t = uint32_t;

            /**
             * Constructor - from the config, the accessOffset will be used.
             */
            ROYALE_API SpiGenericFlash (const royale::config::FlashMemoryConfig &config,
                                        std::shared_ptr<royale::pal::ISpiBusAccess> access,
                                        const royale::common::SensorRoutingConfigSpi &busAddress);
            ~SpiGenericFlash() override;

            void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) override;
            void writeSectorBased (std::size_t startAddr, const std::vector<uint8_t> &buffer) override;

        private:
            std::shared_ptr<royale::pal::ISpiBusAccess> m_access;
            /**
             * Which device on the bus to access.
             *
             * \todo ROYAL-3713 encapsulate with an SPI equivalent of the II2cDeviceAccess.
             */
            royale::common::SensorRoutingConfigSpi m_busAddress;

            /**
             * The startAddr of readStorage, and the zero for writeStorage, are mapped to this
             * offset in the underlying storage.
             */
            device_spi_size_t m_accessOffset;

            /**
             * Size of the storage area
             */
            std::size_t m_imageSize;

            void readStorageBlock (device_spi_size_t startAddr, std::vector<uint8_t> &buffer);
            void writeStorageBlock (device_spi_size_t startAddr, const std::vector<uint8_t> &buffer);

            /**
             * Poll the device's BUSY flag, which will be set while a write or an erase is in
             * progress.
             */
            void waitForWriteComplete();
        };
    }
}
