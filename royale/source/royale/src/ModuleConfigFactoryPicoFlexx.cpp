/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryPicoFlexx.hpp>

#include <modules/ModuleConfigData.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    /**
     * Identifiers for the standard pico flexx and the 15 MHz variant.
     * The pico flexx devices are marked for both CE and FCC conformance.
     * Royale v3.8.0 and earlier will ignore the variant identifier, and
     * some existing devices may have unexpected variant identifiers.
     * Therefore every variant listed in this file must be eye-safe with all
     * hardware that uses the pico flexx's USB VID/PID combination.
     */
    const auto flexxModules = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
                              {
                                  // standard pico flexx
    {
        {0, 0, 0, 0},
        moduleconfig::PicoFlexxU6
    },
                              };

    const auto flexxRouting = std::shared_ptr<ISensorRoutingConfig> (nullptr);
    const auto flexxConfig = FlashMemoryConfig
                             {
                                 FlashMemoryConfig::FlashMemoryType::PICO_PAGED
                             }
                             .setImageSize (2000000)
                             .setPageSize (256);
    const auto flexxDefault = royale::Vector<uint8_t>
                              {
                                  0, 0, 0, 0
                              };
}

ModuleConfigFactoryPicoFlexx::ModuleConfigFactoryPicoFlexx() :
    ModuleConfigFactoryByStorageId (flexxRouting, flexxConfig, flexxModules, flexxDefault)
{
}
