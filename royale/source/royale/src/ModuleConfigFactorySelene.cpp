/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactorySelene.hpp>

#include <modules/ModuleConfigData.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto seleneModules = royale::Vector<ModuleConfigFactoryByStorageIdBase::value_type>
                               {
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::SeleneDefault
    },
    {
        // Selene ICM
        // Module Code : 0300-0000-0000-0000-0000-0000-0000-0004
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 },
        moduleconfig::SeleneIcm
    },

                               };

    const auto seleneRouting = std::make_shared<SensorRoutingImagerAsBridge> (ImagerAsBridgeType::M2453_SPI);

    const auto seleneConfig = FlashMemoryConfig
                              {
                                  FlashMemoryConfig::FlashMemoryType::ZWETSCHGE
                              }
                              .setImageSize (128 * 1024 * 2)
                              .setPageSize (256)
                              .setAccessOffset (0)
                              .setUseCaching (true);

    const auto seleneDefault = royale::Vector<uint8_t>
                               {
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                               };
}

ModuleConfigFactorySelene::ModuleConfigFactorySelene() :
    ModuleConfigFactoryZwetschge (seleneRouting, seleneConfig, seleneModules, seleneDefault)
{
}

