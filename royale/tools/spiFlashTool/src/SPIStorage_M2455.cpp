/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIStorage_M2455.hpp>

#include <common/exceptions/RuntimeError.hpp>
#include <storage/SpiBusMasterM2455.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;
using namespace spiFlashTool;
using namespace spiFlashTool::storage;

namespace
{
    /**
     * The M2455 A11's PRODUCT_CODE register wasn't updated from the M2453, so PRODUCT_CODE can't
     * be used for recognising which device is connected. This register is checked instead of the
     * product code.
     */
    const uint16_t ANAIP_PLL_RAMP_TIME = 0xA037;
    const uint16_t DESIGNSTEP = 0xA0A5;

    std::shared_ptr<royale::pal::ISpiBusAccess> createSpiAccess (const std::shared_ptr<BridgeImagerL4> &access)
    {
        return std::make_shared<SpiBusMasterM2455> (access, royale::config::ImagerAsBridgeType::M2455_SPI);
    }

    const auto BUS_ADDRESS = SensorRoutingConfigSpi
                             {
                                 ONLY_DEVICE_ON_IMAGERS_SPI
                             };
}

SPIStorage_M2455::SPIStorage_M2455 (std::shared_ptr<BridgeImagerL4> access,
                                    IProgressReportListener *progressListener,
                                    std::size_t accessOffset
                                   ) :
    SPIStorageHardwareSpi (createSpiAccess (access), BUS_ADDRESS, progressListener, accessOffset),
    m_access (access)
{
}

SPIStorage_M2455::SPIStorage_M2455 (std::shared_ptr<royale::ICameraDevice> cameraDevice,
                                    IProgressReportListener *progressListener,
                                    std::size_t accessOffset
                                   ) :
    SPIStorage_M2455 (std::make_shared<BridgeImagerL4> (cameraDevice), progressListener, accessOffset)
{
}

royale::Vector<royale::Pair<royale::String, uint64_t>> SPIStorage_M2455::getEFuseRegisters()
{
    uint16_t rampTime;
    m_access->readImagerRegister (ANAIP_PLL_RAMP_TIME, rampTime);
    switch (rampTime)
    {
        case 0x0108:
        case 0x0138:
            // The M2453's default values
            throw RuntimeError ("SPIStorage_M2455 does not support the M2453");
        case 0x0168:
            // The M2455's default value, proceed to reading the design step
            break;
        default:
            LOG (DEBUG) << "ANAIP_PLL_RAMP_TIME: " << rampTime;
            throw RuntimeError ("SPIStorage_M2455 does not recognise the imager type");
    }

    uint16_t designStep;
    m_access->readImagerRegister (DESIGNSTEP, designStep);
    switch (designStep)
    {
        case 0xA11:
            return
            {
                { "0xa097", 0u },
                { "0xa098", 0u },
                { "0xa099", 0u },
                { "0xa09A", 0u }
            };
        default:
            throw RuntimeError ("Design step not supported");
    }
}
