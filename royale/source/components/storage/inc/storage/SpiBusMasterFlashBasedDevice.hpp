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

#include <config/SensorRoutingImagerAsBridge.hpp>
#include <hal/IBridgeImager.hpp>
#include <pal/ISpiBusAccess.hpp>

#include <chrono>
#include <cstdint>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
        * This struct holds the registers needed for the Bus master to work correctly.
        */
        struct SpiRegisters
        {
            uint16_t SPI_WR_ADDR;
            uint16_t SPI_RD_ADDR;
            uint16_t SPI_LEN;
            uint16_t SPI_TRIG;
            uint16_t SPI_STATUS;
            uint16_t SPI_CFG;
        };

        /**
         * With the default ROM firmware, the flash based imagers can act as an SPI bus master, to read and
         * write the calibration storage.  The low-level behavior of the imager is not entirely a
         * generic SPI bus master, as the write-protect feature of the imager means that some
         * commands will be blocked.
         *
         * This class provides the Royale Platform Access Layer for accessing the storage device,
         * an instance of SpiGenericFlash is expected to be constructed on top of this layer.
         */
        class SpiBusMasterFlashBasedDevice : public royale::pal::ISpiBusAccess
        {
        public:
            ROYALE_API SpiBusMasterFlashBasedDevice (std::shared_ptr<royale::hal::IBridgeImager> access);
            ~SpiBusMasterFlashBasedDevice () override;

            // From ISpiBusAccess
            std::unique_lock<std::recursive_mutex> selectDevice (uint8_t id) override;
            ReadSize maximumReadSize () override;
            std::size_t maximumWriteSize () override;
            void readSpi (const std::vector<uint8_t> &transmit, std::vector<uint8_t> &receive) override;
            void writeSpi (const std::vector<uint8_t> &transmit) override;

        protected:
            SpiRegisters m_registers; //has to be filled correctly by the sub classes during construction
            std::shared_ptr<royale::hal::IBridgeImager> m_imager;
            bool m_transmitRequiresReadEnabled = false;

        private:
            /**
             * If the imager is in VIRGIN state, reset the imager so it's in a known state.  After
             * this has reset the imager once, it doesn't reset it again.
             */
            virtual void initializeOnce() = 0;

            /**
             * Set up the data that needs to be sent, set up the read registers if needed, and do
             * the transfer, including waiting for it to complete.
             *
             * If receiveSize is non-zero, then the MISO data after a read-delay corresponding to
             * the transmit size will be stored in the imager's registers, starting at READ_BUFFER
             * (which is defined in the .cpp file).  The READ_BUFFER is not transferred from the
             * imager to Royale by this function, the caller must handle that.
             *
             * If receiveSize is zero then the effect on the READ_BUFFER is implementation-defined.
             */
            void doTransfer (const std::vector<uint8_t> &transmit, std::size_t receiveSize);

            /**
             * This will unconditionally sleep for firstSleep (configured in the .cpp file), and
             * then in a loop read the SPI_STATUS register, with a sleep (configured in the .cpp
             * file) between each retry.
             *
             * \throw RuntimeError if the error bit of the SPI_STATUS register is set.
             * \throw Timeout after a number of retries (the number is defined in the function)
             */
            void waitForTransfer();

            /**
             * Mutex for accessing the storage, for this to be safe it requires that this is the
             * only class that's controlling the imager.
             */
            std::recursive_mutex m_lock;
        };

        /**
         * Argument for the selectDevice() call; currently there's only a single device selectable,
         * and the single constant in the enum is just for documenting that that assumption is being
         * relied on.
         */
        enum SpiDeviceSelectOnImager : uint8_t
        {
            // value is arbitrary, just non-zero to make it different to other enums
            ONLY_DEVICE_ON_IMAGERS_SPI = 0xf0
        };
    }
}
