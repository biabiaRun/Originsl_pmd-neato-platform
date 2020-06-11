/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIStorage_M2453.hpp>

#include <common/exceptions/RuntimeError.hpp>
#include <storage/SpiBusMasterM2453.hpp>
#include <imager/M2453/ImagerRegisters.hpp>

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
        return std::make_shared<SpiBusMasterM2453> (access);
    }

    const auto BUS_ADDRESS = SensorRoutingConfigSpi
                             {
                                 ONLY_DEVICE_ON_IMAGERS_SPI
                             };
}

SPIStorage_M2453::SPIStorage_M2453 (std::shared_ptr<BridgeImagerL4> access,
                                    IProgressReportListener *progressListener,
                                    std::size_t accessOffset
                                   ) :
    SPIStorageHardwareSpi (createSpiAccess (access), BUS_ADDRESS, progressListener, accessOffset),
    m_access (access)
{
}

SPIStorage_M2453::SPIStorage_M2453 (std::shared_ptr<royale::ICameraDevice> cameraDevice,
                                    IProgressReportListener *progressListener,
                                    std::size_t accessOffset
                                   ) :
    SPIStorage_M2453 (std::make_shared<BridgeImagerL4> (cameraDevice), progressListener, accessOffset)
{
}

royale::Vector<royale::Pair<royale::String, uint64_t>> SPIStorage_M2453::getEFuseRegisters()
{
    /**
     * The M2455 A11's PRODUCT_CODE register wasn't updated from the M2453, so PRODUCT_CODE can't
     * be used for recognising which device is connected. This register is checked instead of the
     * product code.
     */
    uint16_t rampTime;
    m_access->readImagerRegister (imager::M2453::ANAIP_PLL_RAMP_TIME, rampTime);
    switch (rampTime)
    {
        case 0x0108:
        case 0x0138:
            // The M2453's default values, proceed to reading the design step
            break;
        case 0x0168:
            // The M2455's default value
            throw RuntimeError ("SPIStorage_M2453 does not support the M2455");
        default:
            LOG (DEBUG) << "ANAIP_PLL_RAMP_TIME: " << rampTime;
            throw RuntimeError ("SPIStorage_M2453 does not recognise the imager type");
    }

    uint16_t designStep;
    m_access->readImagerRegister (imager::M2453::ANAIP_DESIGNSTEP, designStep);
    switch (designStep)
    {
        case 0xA11:
        case 0xA12:
            return
            {
                { toHex (imager::M2453_A11::ANAIP_EFUSEVAL1), 0u },
                { toHex (imager::M2453_A11::ANAIP_EFUSEVAL2), 0u },
                { toHex (imager::M2453_A11::ANAIP_EFUSEVAL3), 0u },
                { toHex (imager::M2453_A11::ANAIP_EFUSEVAL4), 0u }
            };
        case 0xB11:
        case 0xB12:
            return
            {
                { toHex (imager::M2453_B11::ANAIP_EFUSEVAL1), 0u },
                { toHex (imager::M2453_B11::ANAIP_EFUSEVAL2), 0u },
                { toHex (imager::M2453_B11::ANAIP_EFUSEVAL3), 0u },
                { toHex (imager::M2453_B11::ANAIP_EFUSEVAL4), 0u }
            };
        default:
            throw RuntimeError ("Design step not supported");
    }
}
