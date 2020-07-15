/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryPmdModule287x.hpp>

#include <modules/ModuleConfigData.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto modules = royale::Vector<ModuleConfigFactoryByStorageIdBase::value_type>
                         {
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::PmdModule287xDefault
    },
                         };

    const auto routing = std::make_shared<SensorRoutingImagerAsBridge> (ImagerAsBridgeType::M2457_SPI);

    const auto memoryConfig = FlashMemoryConfig
                              {
                                  FlashMemoryConfig::FlashMemoryType::ZWETSCHGE
                              }
                              .setImageSize (128 * 1024 * 32)
                              .setPageSize (256)
                              .setAccessOffset (0)
                              .setUseCaching (true);

    const auto moduleDefault = royale::Vector<uint8_t>
                               {
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                               };
}

ModuleConfigFactoryPmdModule287x::ModuleConfigFactoryPmdModule287x() :
    ModuleConfigFactoryZwetschge (routing, memoryConfig, modules, moduleDefault)
{
}
