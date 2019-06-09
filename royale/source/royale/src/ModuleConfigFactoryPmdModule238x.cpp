/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryPmdModule238x.hpp>

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
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E },
        moduleconfig::Orat45
    },
    {
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30 },
        moduleconfig::MTT016
    },
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::PmdModule238xDefault
    },
                         };

    const auto routing = std::make_shared<SensorRoutingImagerAsBridge> (ImagerAsBridgeType::M2453_SPI);

    const auto moduleConfig = FlashMemoryConfig
                              {
                                  FlashMemoryConfig::FlashMemoryType::ZWETSCHGE
                              }
                              .setImageSize (128 * 1024 * 2)
                              .setPageSize (256)
                              .setAccessOffset (0)
                              .setUseCaching (true);

    const auto moduleDefault = royale::Vector<uint8_t>
                               {
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                               };
}

ModuleConfigFactoryPmdModule238x::ModuleConfigFactoryPmdModule238x() :
    ModuleConfigFactoryZwetschge (routing, moduleConfig, modules, moduleDefault)
{
}
