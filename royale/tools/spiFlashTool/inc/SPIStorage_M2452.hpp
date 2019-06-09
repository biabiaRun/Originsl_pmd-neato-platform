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

#include <SPIStorageBase.hpp>

namespace spiFlashTool
{
    namespace storage
    {
        /**
         * With the special firmware in the ImagerSPIFirmware.hpp file, the imager will act as an
         * SPI bus master, and read and write the calibration storage.
         *
         * This class provides the Royale Platform Access Layer for accessing the storage device,
         * and can have StorageFormatPolar constructed on top of this layer.
         */
        class SPIStorage_M2452 : public SPIStorageBase
        {
        public:
            SPIStorage_M2452 (std::shared_ptr<royale::ICameraDevice> cameraDevice,
                              IProgressReportListener *progressListener = nullptr);

            void loadFirmware();
            /**
             * If the firmware isn't already loaded, load it.
             *
             *\todo: what access patterns will this storage class have to support?
             */
            void loadFirmwareOnce();
            void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) override;
            void writeStorage (const std::vector<uint8_t> &buffer) override;
            royale::Vector<royale::Pair<royale::String, uint64_t>> getEFuseRegisters() override;

        private:
            std::shared_ptr<royale::ICameraDevice> m_cameraDevice;
            bool m_firmwareLoaded = false;

            /**
             * All of the commands are started by packing the command and 24-bit address in to a
             * pair of 16-bit registers. This packs the data and sends it.
             */
            royale::CameraStatus writeCommandAndAddress (uint8_t command, std::size_t address);
        };
    }
}
