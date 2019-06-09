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

#include <common/ISensorRoutingConfig.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <factory/ModuleConfigFactoryByStorageIdBase.hpp>

namespace royale
{
    namespace factory
    {
        // If no Zwetschge file is found on the flash or if the CRCs match
        // use the Zwetschge information from the default file
        const royale::String ZWETSCHGE_BACKUP_FILE ("ZwetschgeDefault.zwetschge");


        /**
         * A factory which loads Zwetschge from the device's flash during the probe phase, and
         * merges the data in the external config (the Zwetschge data) in to the ModuleConfig.
         *
         * This uses ModuleConfigFactoryByStorageIdBase's handling for caching the calibration.
         * If the ModuleConfig is configured to use FlashMemoryType::FIXED but with a nullptr in
         * FlashMemoryConfig::nonVolatileStorageFixed then a NonVolatileStorageShadow will be
         * created which caches the calibration data from the device's Zwetschge storage; this is
         * the recommended configuration for devices using Zwetschge.
         *
         * The probeAndCreate() function will not retain any references to the IBridgeFactory or to
         * access methods derived from the IBridgeFactory; for example it will not put an
         * INonVolatileStorage in to the ModuleConfig that requires SPI-via-Imager.  If the
         * recommended caching mechanism is not used, it is unlikely (but implementation-defined)
         * whether the calibration data will be available via the ModuleConfig.
         *
         * @inheritdoc
         */
        class ModuleConfigFactoryZwetschge : public ModuleConfigFactoryByStorageIdBase
        {
        public:
            using value_type = ModuleConfigFactoryByStorageIdBase::value_type;

            ROYALE_API ModuleConfigFactoryZwetschge (
                const std::shared_ptr<royale::common::ISensorRoutingConfig> &route,
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
