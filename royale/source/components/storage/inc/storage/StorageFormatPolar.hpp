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

#include <pal/IStorageAccessUnderlying.hpp>
#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <hal/INonVolatileStorage.hpp>

#include <royale/Definitions.hpp>

#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * A format for the non-volatile storage that holds the calibration data.
         *
         * The name Polar is based on the UVC extension's name (Arctic), but this class can be used
         * with any storage that contains data in the right format and supports IStorageReadRandom.
         */
        class StorageFormatPolar : public royale::hal::INonVolatileStorage,
            public royale::pal::IStorageAccessUnderlying
        {
        public:
            /**
             * If the storageAccess also implements IStorageWriteFullOverwrite then the
             * memory can also be written to.  If it does not then writeCalibrationData
             * will throw an exception.
             *
             * If identifierIsMandatory is true then only the three-argument version of
             * writeCalibrationData can be used, the single-argument version of writeCalibrationData
             * will throw without writing to the device.
             */
            ROYALE_API StorageFormatPolar (std::shared_ptr<royale::pal::IStorageReadRandom> storageAccess,
                                           bool identifierIsMandatory = false);

            ROYALE_API virtual ~StorageFormatPolar() override = default;

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

            ROYALE_API std::shared_ptr<pal::IStorageWriteFullOverwrite> getUnderlyingWriteAccess() override;

        private:
            struct FlashHeader;
            /**
             * Reads the data via the Bridge, and parses it in to a FlashHeader.  The parsed
             * data returned is in native endianness and alignment. If the magic PMD entry was
             * not found, false is returned. This means no valid PMD data available.
             *
             * \param header output for where the data is stored
             * \return true if the magic PMD information was found and header is ok
             */
            bool readHeaderFromEEPROM (struct FlashHeader &header);

            /**
             * Common implementation of both writeCalibrationData functions.  For the pointer
             * arguments, either all of them are nullptr or all of them must be non-null.
             */
            void writeV3orV7 (const royale::Vector<uint8_t> &data,
                              const royale::Vector<uint8_t> *identifier,
                              const royale::String *suffix,
                              const royale::String *serialNumber);

            std::shared_ptr<pal::IStorageReadRandom> m_bridge;

            /** If true, only the 3-argument version of writeCalibrationData can be used. */
            bool m_identifierIsMandatory;
        };
    }
}
