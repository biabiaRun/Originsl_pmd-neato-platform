/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <config/CoreConfig.hpp>
#include <config/ModuleConfig.hpp>

#include <factory/ICameraCoreBuilder.hpp>
#include <factory/IProcessingParameterMapFactory.hpp>
#include <factory/CameraCoreBuilderImpl.hpp>
#include <common/MakeUnique.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * Factory that creates an ICoreConfig (with all the right sizes for use case images,
         * processing parameters, etc) from a CoreConfig.
         *
         * The CoreConfig that is passed to either of the constructors can be, as documented in
         * CoreConfig.hpp, one that does not have sizes for the use case images.
         */
        class CoreConfigFactory
        {
        public:
            ROYALE_API explicit CoreConfigFactory (const royale::config::CoreConfig &data,
                                                   std::shared_ptr<royale::factory::IProcessingParameterMapFactory> factory);

            ROYALE_API explicit CoreConfigFactory (const royale::config::ModuleConfig &data,
                                                   std::shared_ptr<royale::factory::IProcessingParameterMapFactory> factory);

            /**
             * Resolve internal data, and return an ICoreConfig.
             *
             * This will throw if required data is missing or another error is detected. For
             * example, multiple use cases having the same name with cause this to throw.
             *
             * @throw LogicError if there is missing or inconsistent data
             */
            ROYALE_API std::unique_ptr<royale::config::ICoreConfig> operator() ();

        private:
            const royale::config::CoreConfig &m_data;
            const std::shared_ptr<royale::factory::IProcessingParameterMapFactory> m_paramFactory;
        };
    }
}
