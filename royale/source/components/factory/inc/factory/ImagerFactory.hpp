/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <memory>
#include <common/IPseudoDataInterpreter.hpp>
#include <config/ICoreConfig.hpp>
#include <config/IlluminationConfig.hpp>
#include <config/ImagerConfig.hpp>
#include <config/ImagerType.hpp>
#include <hal/IBridgeImager.hpp>
#include <hal/IImager.hpp>

namespace royale
{
    namespace factory
    {
        class ImagerFactory
        {
        public:

            /**
            * Retrieves the pseudo data interpreter string based on a given ImagerType.
            * @param imagerType imagerType that should be used
            */
            static royale::String getImagerTypeName (config::ImagerType imagerType);

            /**
            * Retrieves the pseudo data interpreter based on a given ImagerType.
            * @param imagerType imagerType that should be used
            */
            static royale::String getPseudoDataInterpreter (config::ImagerType imagerType);

            /**
            * Creates a pseudo data interpreter based on a given string.
            * @param pseudoDataInterpreter pseudoDataInterpreter that should be used
            */
            static std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter (const royale::String &pseudoDataInterpreter);

            /**
            * Creates a pseudo data interpreter based on the given ImagerType.
            * @param imagerType imager type of the module
            */
            static std::unique_ptr<common::IPseudoDataInterpreter> createPseudoDataInterpreter (config::ImagerType imagerType);

            /**
            * Returns true if this type of imager requires its use case definitions to have unique
            * identifiers, and returns false if it doesn't.
            *
            * This symbol is exported, so that unit tests can use it to sanity-check that module
            * configs match the imager type.
            *
            * @param imagerType type of imager whose specification is requested
            */
            ROYALE_API static bool getRequiresUseCaseDefGuids (config::ImagerType imagerType);

            /**
            * Creates an imager instance based on the given ImagerType.
            * @param bridge an instance of an IBridgeImager
            * @param coreConfig CoreConfig of the camera module
            * @param imagerConfig Imager configuration data (including imager type and base config)
            * @param illuminationConfig configuration data for the illumination unit
            * @param directAccessEnabled create an imager that allows to read and write registers
            */
            static std::shared_ptr<hal::IImager> createImager (
                std::shared_ptr<royale::hal::IBridgeImager> bridge,
                const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
                const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
                const royale::config::IlluminationConfig &illuminationConfig,
                bool directAccessEnabled);
        };
    }
}
