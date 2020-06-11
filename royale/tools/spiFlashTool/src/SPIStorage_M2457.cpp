/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIStorage_M2457.hpp>

#include <common/exceptions/RuntimeError.hpp>
#include <storage/SpiBusMasterM2457.hpp>
#include <imager/M2457/ImagerRegisters.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;
using namespace spiFlashTool;
using namespace spiFlashTool::storage;

namespace
{
    std::shared_ptr<royale::pal::ISpiBusAccess> createSpiAccess (const std::shared_ptr<BridgeImagerL4> &access)
    {
        return std::make_shared<SpiBusMasterM2457> (access);
    }

    const auto BUS_ADDRESS = SensorRoutingConfigSpi
                             {
                                 ONLY_DEVICE_ON_IMAGERS_SPI
                             };
}

SPIStorage_M2457::SPIStorage_M2457 (std::shared_ptr<BridgeImagerL4> access,
                                    IProgressReportListener *progressListener,
                                    std::size_t accessOffset
                                   ) :
    SPIStorageHardwareSpi (createSpiAccess (access), BUS_ADDRESS, progressListener, accessOffset),
    m_access (access)
{
}

SPIStorage_M2457::SPIStorage_M2457 (std::shared_ptr<royale::ICameraDevice> cameraDevice,
                                    IProgressReportListener *progressListener,
                                    std::size_t accessOffset
                                   ) :
    SPIStorage_M2457 (std::make_shared<BridgeImagerL4> (cameraDevice), progressListener, accessOffset)
{
}

royale::Vector<royale::Pair<royale::String, uint64_t>> SPIStorage_M2457::getEFuseRegisters()
{
    uint16_t productCode;
    m_access->readImagerRegister (imager::M2457::ANAIP_PRODUCTCODE, productCode);
    switch (productCode)
    {
        case 0x2877:
            // The M2457's default value, proceed to reading the design step
            break;
        default:
            LOG (DEBUG) << "Product code : " << productCode;
            throw RuntimeError ("SPIStorage_M2457 does not recognise the imager type");
    }

    uint16_t designStep;
    m_access->readImagerRegister (imager::M2457::ANAIP_DESIGNSTEP, designStep);
    switch (designStep)
    {
        case 0xA11:
            return
            {
                { toHex (imager::M2457_A11::ANAIP_EFUSEVAL1), 0u },
                { toHex (imager::M2457_A11::ANAIP_EFUSEVAL2), 0u },
                { toHex (imager::M2457_A11::ANAIP_EFUSEVAL3), 0u },
                { toHex (imager::M2457_A11::ANAIP_EFUSEVAL4), 0u }
            };
        default:
            throw RuntimeError ("Design step not supported");
    }
}