/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <factory/IModuleConfigFactory.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * A factory which returns a fixed ModuleConfig.
         *
         * The ModuleConfig to be returned is passed on construction.
         * No probing is done, the bridgeFactory argument to
         * probeAndCreate is ignored.
         *
         */
        class ModuleConfigFactoryFixed : public IModuleConfigFactory
        {
        public:
            ROYALE_API explicit ModuleConfigFactoryFixed (const royale::config::ModuleConfig &moduleConfig);

            std::shared_ptr<const royale::config::ModuleConfig>
            probeAndCreate (royale::factory::IBridgeFactory &bridgeFactory) const override;
            royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> enumerateConfigs() const override;
        private:
            std::shared_ptr<const royale::config::ModuleConfig> m_moduleConfig;
        };
    }
}
