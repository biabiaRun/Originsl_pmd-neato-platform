/****************************************************************************\
 * Copyright (C) 2020 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryX1.hpp>

#include <modules/ModuleConfigData.hpp>
#include <common/SensorRoutingConfigI2c.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto x1Modules = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
                           {
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::X1Default
    },
    {
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33 },
        moduleconfig::X1_18502W
    },
    {
        { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x71 },
        moduleconfig::X1_2
    },
                           };

    const auto x1Routing = std::make_shared<SensorRoutingConfigI2c> (0x56);
    const auto x1Config = FlashMemoryConfig
                          {
                              FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
                          }
                          .setImageSize (128 * 1024)
                          .setPageSize (256)
                          .setWriteTime (std::chrono::microseconds{ 5000 });
    const auto x1Default = royale::Vector<uint8_t>
                           {
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                           };
}

ModuleConfigFactoryX1::ModuleConfigFactoryX1() :
    ModuleConfigFactoryByStorageId (x1Routing, x1Config, x1Modules, x1Default)
{
}
