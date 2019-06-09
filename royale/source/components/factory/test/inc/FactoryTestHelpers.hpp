/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <config/ModuleConfig.hpp>
// NVSF.hpp includes all dependencies of the StructForCreateSimBridgeImagerFactoryWithStorage
#include <factory/NonVolatileStorageFactory.hpp>

namespace royale
{
    namespace test
    {
        namespace utils
        {
            /**
             * Returns a ModuleConfig that's sufficient for the factory unit tests,
             * with default values for everything that has default values.
             */
            royale::config::ModuleConfig getMinimalModuleConfig();

            /**
             * See createSimBridgeImagerFactoryWithStorage.
             */
            struct StructForCreateSimBridgeImagerFactoryWithStorage
            {
                std::shared_ptr<std::map<uint32_t, uint8_t>> simulatedSpiStorage;
                std::shared_ptr<royale::factory::IBridgeFactory> bridgeFactory;
                std::shared_ptr<royale::common::ISensorRoutingConfig> routing;
            };

            /**
             * Returns a simulated image with attached SPI storage, and with a copy of the given
             * data in that storage.
             *
             * The returned structure contains a bridgeFactory and routing from which
             * NonVolatileStorageFactory::createStorageReadRandom can create an IStorageReadRandom
             * pointing to the data in simulatedSpiStorage.
             */
            StructForCreateSimBridgeImagerFactoryWithStorage createSimBridgeImagerFactoryWithStorage (const std::vector<uint8_t> &flashContents);
        }
    }
}
