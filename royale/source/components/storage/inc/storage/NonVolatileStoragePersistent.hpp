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

#include <royale/Definitions.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <memory>

namespace royale
{
    namespace storage
    {
        /**
        * This class provides access to calibration files on persistent storage.
        */
        class NonVolatileStoragePersistent : public royale::hal::INonVolatileStorage
        {
        public:
            ROYALE_API explicit NonVolatileStoragePersistent (const royale::String &fileName);
            ROYALE_API ~NonVolatileStoragePersistent() override = default;

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
            royale::String m_fileName;
        };
    }
}
