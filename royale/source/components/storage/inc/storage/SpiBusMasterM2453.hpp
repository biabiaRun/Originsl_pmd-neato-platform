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
         * With the default ROM firmware, the M2453 imager can act as an SPI bus master, to read and
         * write the calibration storage.  The low-level behavior of the imager is not entirely a
         * generic SPI bus master, as the write-protect feature of the imager means that some
         * commands will be blocked.
         *
         * The M2455 is also supported, there is a slight difference between the two so the correct
         * ImagerAsBridgeType must be passed to the constructor.
         *
         * This class provides the Royale Platform Access Layer for accessing the storage device,
         * an instance of SpiGenericFlash is expected to be constructed on top of this layer.
         */
        class SpiBusMasterM2453 : public royale::pal::ISpiBusAccess
        {
        public:
            ROYALE_API SpiBusMasterM2453 (std::shared_ptr<royale::hal::IBridgeImager> access,
                                          royale::config::ImagerAsBridgeType firmwareType);
            ~SpiBusMasterM2453 () override;

            // From ISpiBusAccess
            std::unique_lock<std::recursive_mutex> selectDevice (uint8_t id) override;
            ReadSize maximumReadSize () override;
            std::size_t maximumWriteSize () override;
            void readSpi (const std::vector<uint8_t> &transmit, std::vector<uint8_t> &receive) override;
            void writeSpi (const std::vector<uint8_t> &transmit) override;

        private:
            /**
             * If the imager is in VIRGIN state, reset the imager so it's in a known state.  After
             * this has reset the imager once, it doesn't reset it again.
             */
            void initializeOnce();

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
            std::shared_ptr<royale::hal::IBridgeImager> m_imager;

            bool m_transmitRequiresReadEnabled = false;
        };

        /**
         * Argument for the selectDevice() call; currently there's only a single device selectable,
         * and the single constant in the enum is just for documenting that that assumption is being
         * relied on.
         *
         * This may move to a different header in future, as it will be used for both M2453 and
         * M2455, however wherever it moves to is likely to be indirectly included by including
         * either SpiBusMasterM2453.hpp or SpiBusMasterM2455.hpp.
         */
        enum SpiDeviceSelectOnImager : uint8_t
        {
            // value is arbitrary, just non-zero to make it different to other enums
            ONLY_DEVICE_ON_IMAGERS_SPI = 0xf0
        };
    }
}
