/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactoryAnimatorBoard.hpp>

#include <modules/ModuleConfigData.hpp>
#include <common/SensorRoutingConfigSpi.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    /**
     * This ID can never be read from the device (if there is an ID, it will be longer than this),
     * it's simply used as the default to map all unrecognised IDs to the default config.
     *
     * This factory is only used for boards with the Animator bring-up firmware, so it will only be
     * used for development and evaluation boards.  The developer is responsible for their own eye
     * safety, and ensuring that devices only have identifiers which load matching configurations.
     */
    const auto PLACEHOLDER_ID = royale::Vector<uint8_t>
                                {
                                    0
                                };

    const auto animatorModules = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
                                 {
                                     // Default case
                                     {
                                         PLACEHOLDER_ID,
                                         moduleconfig::AnimatorDefault
                                     }
                                 };

    const auto animatorRouting = std::make_shared<SensorRoutingConfigSpi> (0x0);
    const auto animatorConfig = FlashMemoryConfig
                                {
                                    FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
                                }
                                .setPageSize (256)
                                .setAccessOffset (0x30000);
}

ModuleConfigFactoryAnimatorBoard::ModuleConfigFactoryAnimatorBoard() :
    ModuleConfigFactoryByStorageId (animatorRouting, animatorConfig, animatorModules, PLACEHOLDER_ID)
{
}
