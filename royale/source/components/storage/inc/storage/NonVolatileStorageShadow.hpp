/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * Container for calibration data that already has been read out, where
         * the actual storage can or should not be accessed again.
         *
         * This container is immutable, writeCalibrationData will simply throw.
         */
        class NonVolatileStorageShadow : public royale::hal::INonVolatileStorage
        {
        public:
            ROYALE_API NonVolatileStorageShadow (const royale::Vector<uint8_t> &calibrationData,
                                                 const royale::Vector<uint8_t> &identifier = {},
                                                 const royale::String &suffix = "",
                                                 const royale::String &serialNumber = "");

            /**
             * Create a cached copy of any INonVolatileStorage.
             * If useCaching is enabled, the constructor will first try to load a
             * serialNumber.cal file from the working path before accessing the
             * src object. Should there be no cached copy of the calibration
             * it will access the storage to download the calibration and save
             * it to the working path.
             * The serial number that is used for the creation of the filename
             * is taken from the storage. If the serial number is empty caching
             * will be disabled.
             */
            explicit ROYALE_API NonVolatileStorageShadow (royale::hal::INonVolatileStorage &src,
                    bool useCaching = false);

            ROYALE_API royale::Vector<uint8_t> getModuleIdentifier() override;
            ROYALE_API royale::String getModuleSuffix() override;
            ROYALE_API royale::String getModuleSerialNumber() override;
            ROYALE_API royale::Vector<uint8_t> getCalibrationData() override;
            ROYALE_API uint32_t getCalibrationDataChecksum() override;

            ROYALE_API void writeCalibrationData (const royale::Vector<uint8_t> &data) override;
            ROYALE_API void writeCalibrationData (const royale::Vector<uint8_t> &data,
                                                  const royale::Vector<uint8_t> &identifier,
                                                  const royale::String &suffix,
                                                  const royale::String &serialNumber) override;

        private:
            royale::Vector<uint8_t> m_calibrationData;
            const royale::Vector<uint8_t> m_identifier;
            const royale::String m_suffix;
            const royale::String m_serialNumber;
            const uint32_t m_calibrationDataChecksum;
        };
    }
}
