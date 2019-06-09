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

#include <SPIStorageBase.hpp>
#include <pal/ISpiBusAccess.hpp>
#include <storage/SpiGenericFlash.hpp>

namespace spiFlashTool
{
    namespace storage
    {
        /**
         * This implements common functionality for the M2453 and M2455 imagers, which do not need
         * additional firmware to act as an SPI bus master, and which all have similar SPI support.
         */
        class SPIStorageHardwareSpi : public SPIStorageBase
        {
        public:
            SPIStorageHardwareSpi (std::shared_ptr<royale::pal::ISpiBusAccess> spiAccess,
                                   const royale::common::SensorRoutingConfigSpi &busAddress,
                                   IProgressReportListener *progressListener,
                                   std::size_t accessOffset);

            void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) override;
            void writeStorage (const std::vector<uint8_t> &buffer) override;

            void writeStorage (std::size_t startAddr, const std::vector<uint8_t> &buffer);

        private:
            void waitForTransfer();
            std::shared_ptr<royale::storage::SpiGenericFlash> m_flash;
        };
    }
}
