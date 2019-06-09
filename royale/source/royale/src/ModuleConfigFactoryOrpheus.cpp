/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryOrpheus.hpp>

#include <modules/ModuleConfigData.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto orpheusModules = royale::Vector<ModuleConfigFactoryByStorageIdBase::value_type>
                                {
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::OrpheusDefault
    },
    {
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09 },
        moduleconfig::Orpheus
    },
                                };

    const auto orpheusRouting = std::make_shared<SensorRoutingImagerAsBridge> (ImagerAsBridgeType::M2453_SPI);

    const auto orpheusConfig = FlashMemoryConfig
                               {
                                   FlashMemoryConfig::FlashMemoryType::ZWETSCHGE
                               }
                               .setImageSize (128 * 1024 * 2)
                               .setPageSize (256)
                               .setAccessOffset (0)
                               .setUseCaching (true);

    const auto orpheusDefault = royale::Vector<uint8_t>
                                {
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                };
}

ModuleConfigFactoryOrpheus::ModuleConfigFactoryOrpheus() :
    ModuleConfigFactoryZwetschge (orpheusRouting, orpheusConfig, orpheusModules, orpheusDefault)
{
}

