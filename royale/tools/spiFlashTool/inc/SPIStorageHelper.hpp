/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <SPIStorageHardwareSpi.hpp>

namespace spiFlashTool
{
    namespace storage
    {
        std::shared_ptr<spiFlashTool::storage::SPIStorageHardwareSpi> createSPIStorage (
            std::shared_ptr<royale::ICameraDevice> cameraDevice,
            IProgressReportListener *progressListener = nullptr,
            std::size_t accessOffset = 0x2000);
    }
}
