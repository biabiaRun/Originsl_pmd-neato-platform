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

#include <hal/INonVolatileStorage.hpp>
#include <pal/IStorageReadRandom.hpp>

#include <usb/factory/IBridgeFactory.hpp>
#include <common/ISensorRoutingConfig.hpp>
#include <config/ExternalConfig.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <config/ImagerConfig.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        /**
         * Options for NonVolatileStorageFactory, may change the list of storage devices that can be
         * constructed.
         */
        enum class NonVolatileStorageFactoryConstraint
        {
            NONE,
            /**
             * The caller will ensure that the storage is only used during a time when the caller
             * has exclusive access to the module, and that it is destroyed before other parts of
             * Royale can access the module.  This constraint is required for the StorageSpiImager,
             * which would cause incorrect operation if an IImager implementation had simultaneous
             * access to the module.
             */
            CONTROLLED_LIFETIME
        };

        class NonVolatileStorageFactory
        {
        public:
            ROYALE_API static std::shared_ptr<royale::hal::INonVolatileStorage> createFlash (
                royale::factory::IBridgeFactory &bridgeFactory,
                const royale::config::FlashMemoryConfig  &flashMemoryConfig,
                const royale::common::ISensorRoutingConfig *sensorRouting,
                NonVolatileStorageFactoryConstraint = NonVolatileStorageFactoryConstraint::NONE);

            /**
             * Subfunction of createFlash, throws if it can't create an IStorageReadRandom.
             */
            ROYALE_API static std::shared_ptr<royale::pal::IStorageReadRandom> createStorageReadRandom (
                royale::factory::IBridgeFactory &bridgeFactory,
                const royale::config::FlashMemoryConfig  &flashMemoryConfig,
                const royale::common::ISensorRoutingConfig *sensorRouting,
                NonVolatileStorageFactoryConstraint = NonVolatileStorageFactoryConstraint::NONE);

            /**
             * Returns an ExternalConfig object (not a shared_ptr to it, as the ExternalConfig
             * itself is just a vector and a pair of shared_ptrs).
             *
             * This function will only load the ExternalConfig from a file (Lena or Zwetschge),
             * or a built-in resource (created with ExternalConfigFileConfig::fromLenaString).
             */
            ROYALE_API static royale::config::ExternalConfig createExternalConfig (
                const royale::config::ExternalConfigFileConfig &storageConfig);

            /**
             * Returns an ExternalConfig from any storage supported by createStorageReadRandom.
             */
            ROYALE_API static royale::config::ExternalConfig createExternalConfig (
                royale::factory::IBridgeFactory &bridgeFactory,
                const royale::config::FlashMemoryConfig  &flashMemoryConfig,
                const royale::common::ISensorRoutingConfig *sensorRouting,
                NonVolatileStorageFactoryConstraint = NonVolatileStorageFactoryConstraint::NONE);
        }; // class NonVolatileStorageFactory

    } // namespace factory

} // namespace royale
