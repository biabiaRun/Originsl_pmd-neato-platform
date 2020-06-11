/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryPicoMonstar.hpp>

#include <modules/ModuleConfigData.hpp>
#include <common/SensorRoutingConfigSpi.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto picomonstarModules = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
                                    {
                                        // Default case
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::PicoMonstarDefault
    },
    // Pico monstar with glass lens (2019 batch)
    // Product code (hex) : 0x03 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x2D
    {
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2D },
        moduleconfig::PicoMonstar850nmGlass2
    },
                                    };

    const auto picomonstarRouting = std::make_shared<SensorRoutingConfigSpi> (0x0);
    const auto picomonstarConfig = FlashMemoryConfig
                                   {
                                       FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
                                   }
                                   .setPageSize (256)
                                   .setSectorSize (0x10000)
                                   .setAccessOffset (0x20000);
    const auto picomonstarDefault = royale::Vector<uint8_t>
                                    {
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                    };

}

ModuleConfigFactoryPicoMonstar::ModuleConfigFactoryPicoMonstar() :
    ModuleConfigFactoryByStorageId (picomonstarRouting, picomonstarConfig, picomonstarModules, picomonstarDefault)
{
}
