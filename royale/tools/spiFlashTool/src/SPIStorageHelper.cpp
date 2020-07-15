/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIStorageHelper.hpp>

#include <BridgeImagerL4.hpp>
#include <RoyaleLogger.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <SPIStorage_M2453.hpp>
#include <SPIStorage_M2455.hpp>
#include <SPIStorage_M2457.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2455/ImagerRegisters.hpp>
#include <imager/M2457/ImagerRegisters.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;
using namespace spiFlashTool;
using namespace spiFlashTool::storage;

namespace spiFlashTool
{
    namespace storage
    {

        std::shared_ptr<spiFlashTool::storage::SPIStorageHardwareSpi> createSPIStorage (
            std::shared_ptr<royale::ICameraDevice> cameraDevice,
            IProgressReportListener *progressListener,
            std::size_t accessOffset)
        {
            auto bridgeImagerL4 = std::make_shared<BridgeImagerL4> (cameraDevice);

            uint16_t productCode;
            //Make sure the register addresses are the same in all devices to determine the correct product code
            static_assert (imager::M2453::ANAIP_PRODUCTCODE == imager::M2455::ANAIP_PRODUCTCODE, "Product code register addresses differ!");
            static_assert (imager::M2453::ANAIP_PRODUCTCODE == imager::M2457::ANAIP_PRODUCTCODE, "Product code register addresses differ!");
            bridgeImagerL4->readImagerRegister (imager::M2453::ANAIP_PRODUCTCODE, productCode);

            switch (productCode)
            {
                case 0x2381:
                    /**
                    * The M2455 A11's PRODUCT_CODE register wasn't updated from the M2453, so PRODUCT_CODE can't
                    * be used for recognizing which device is connected. This register is checked instead of the
                    * product code.
                    */
                    static_assert (imager::M2453::ANAIP_PLL_RAMP_TIME == imager::M2455::ANAIP_PLL_RAMP_TIME, "PLL Ramp Time register addresses differ!");
                    uint16_t rampTime;
                    bridgeImagerL4->readImagerRegister (imager::M2453::ANAIP_PLL_RAMP_TIME, rampTime);
                    switch (rampTime)
                    {
                        case 0x0108:
                        case 0x0138:
                            // The M2453's default values
                            return std::make_shared<SPIStorage_M2453> (cameraDevice, progressListener, accessOffset);
                        case 0x0168:
                            // The M2455's default value, proceed to reading the design step
                            return std::make_shared<SPIStorage_M2455> (cameraDevice, progressListener, accessOffset);
                        default:
                            LOG (DEBUG) << "ANAIP_PLL_RAMP_TIME: " << rampTime;
                            throw RuntimeError ("SPIStorage_M2455 does not recognize the imager type");
                    }
                case 0x2771:
                    return std::make_shared<SPIStorage_M2455> (cameraDevice, progressListener, accessOffset);
                case 0x2877:
                    return std::make_shared<SPIStorage_M2457> (cameraDevice, progressListener, accessOffset);

                default:
                    throw RuntimeError ("Imager type not supported");
            }
        }

    }
}