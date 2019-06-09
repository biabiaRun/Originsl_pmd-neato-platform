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

#include <common/ISensorRoutingConfig.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <factory/ModuleConfigFactoryByStorageIdBase.hpp>
#include <hal/INonVolatileStorage.hpp>

#include <royale/Vector.hpp>

#include <memory>
#include <utility>

namespace royale
{
    namespace factory
    {
        /**
         * A factory that recognises devices by constructing an INonVolatileStorage from the given
         * IBridgeFactory, ISensorRoutingConfig and FlashMemoryConfig, and then reading the
         * INonVolatileStorage::getModuleIdentifier(). It can optionally also populate a cached copy
         * of the calibration data.
         *
         * @inheritdoc
         */
        class ModuleConfigFactoryByStorageId : public ModuleConfigFactoryByStorageIdBase
        {
        public:
            using value_type = ModuleConfigFactoryByStorageIdBase::value_type;

            ROYALE_API ModuleConfigFactoryByStorageId (const std::shared_ptr<royale::common::ISensorRoutingConfig> &route,
                    const royale::config::FlashMemoryConfig &memoryConfig,
                    const royale::Vector<value_type> configs,
                    const royale::Vector<uint8_t> &defaultId = {});

            std::shared_ptr<const royale::config::ModuleConfig>
            probeAndCreate (royale::factory::IBridgeFactory &bridgeFactory) const override;

        private:
            std::shared_ptr<royale::common::ISensorRoutingConfig> m_route;
            const royale::config::FlashMemoryConfig m_memoryConfig;
        };
    }
}
