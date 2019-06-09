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

#include <BridgeImagerL4.hpp>
#include <SPIStorageHardwareSpi.hpp>

#include <pal/ISpiBusAccess.hpp>
#include <royale/ICameraDevice.hpp>

namespace spiFlashTool
{
    namespace storage
    {
        class SPIStorage_M2455 : public SPIStorageHardwareSpi
        {
        public:
            SPIStorage_M2455 (std::shared_ptr<BridgeImagerL4> access,
                              IProgressReportListener *progressListener = nullptr,
                              std::size_t accessOffset = 0x2000);

            SPIStorage_M2455 (std::shared_ptr<royale::ICameraDevice> cameraDevice,
                              IProgressReportListener *progressListener = nullptr,
                              std::size_t accessOffset = 0x2000);

            royale::Vector<royale::Pair<royale::String, uint64_t>> getEFuseRegisters() override;

        private:
            std::shared_ptr<BridgeImagerL4> m_access;
        };
    }
}
