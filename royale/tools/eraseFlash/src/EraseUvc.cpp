/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <EraseUvc.hpp>
#include <factory/NonVolatileStorageFactory.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <usb/factory/IBridgeFactory.hpp>

#include <common/MakeUnique.hpp>
#include <common/SensorRoutingConfigSpi.hpp>

#include <memory>
#include <stdint.h>
#include <vector>

using namespace royale::common;
using namespace royale::config;
using namespace royale::pal;
using namespace royale::factory;
using namespace eraseflash;

namespace
{
    class ArcticTool : public EraseTool
    {
        // We only have to erase the first few bytes to invalidate the firmware
        // and get to the Bootloader
        const std::size_t BYTES_TO_WIPE = 4;

    public:
        explicit ArcticTool (IBridgeFactory &factory)
        {
            // Ignore the module's normal FlashMemoryConfig, we always want to wipe the zeroth SPI
            // memory, which means we want to wipe one sector.  This config is inaccurate but will
            // work for any SPI (assuming pages of BYTES_TO_WIPE or larger).
            //
            // However, if we wanted to write a new firmware image to the storage we'd need to know
            // the correct values to give to setPageSize and setSectorSize.
            auto routing = SensorRoutingConfigSpi {0};
            auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::NONE}
                          .setPageSize (BYTES_TO_WIPE)
                          .setSectorSize (BYTES_TO_WIPE);
            auto storage = NonVolatileStorageFactory::createStorageReadRandom (factory, config, &routing);
            m_storage = std::dynamic_pointer_cast<IStorageWriteFullOverwrite> (storage);

            if (m_storage == nullptr)
            {
                throw CouldNotOpen();
            }
        }

        ~ArcticTool() = default;

        bool eraseFlash() override
        {
            try
            {
                // We only have to erase the first few bytes to invalidate the firmware
                // and get to the Bootloader
                std::vector<uint8_t> buf (BYTES_TO_WIPE, 0);
                m_storage->writeStorage (buf);
            }
            catch (...)
            {
                return false;
            }
            return true;
        }

    private:
        std::shared_ptr<royale::pal::IStorageWriteFullOverwrite> m_storage;
    };
}

QString EraseToolArcticFactory::getName()
{
    return "UVC (Arctic)";
}

std::unique_ptr<EraseTool> EraseToolArcticFactory::createTool (IBridgeFactory &bridgeFactory)
{
    return makeUnique<ArcticTool> (bridgeFactory);
}
