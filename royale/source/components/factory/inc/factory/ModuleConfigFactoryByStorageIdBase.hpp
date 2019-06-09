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

#include <factory/IModuleConfigFactory.hpp>
#include <hal/INonVolatileStorage.hpp>

#include <royale/Vector.hpp>

#include <memory>
#include <utility>

namespace royale
{
    namespace factory
    {
        /**
         * A factory that recognises devices by INonVolatileStorage::getModuleIdentifier(). It can
         * optionally also populate a cached copy of the calibration data.
         *
         * This is a common base class of ModuleConfigFactoryByStorageId and
         * ModuleConfigFactoryZwetschge. The subclass handles getting the INonVolatileStorage
         * instance, and this base class handles the module lookup using the INonVolatileStorage.
         *
         * The constructor takes a list of (INonVolatileStorage::getModuleIdentifier(),
         * ModuleConfig) pairs, and the result of getModuleIdentifier() is looked up in that list.
         *
         * If defaultId is non-empty and the module isn't recognised from the list, then the list
         * will be searched for the defaultId. If it is found, the corresponding ModuleConfig will
         * be returned, otherwise nullptr will be returned.  Note that the configs list can include
         * a pair that maps an empty identifier to a ModuleConfig, but only a pair that has a
         * non-empty identifier can be selected via the defaultId.
         *
         * This can also read the calibration data from the INonVolatileStorage and store a
         * cached-in-memory copy of it in the ModuleConfig.  To use this feature, configure the
         * ModuleConfig to use FlashMemoryType::FIXED, but with a nullptr in
         * FlashMemoryConfig::nonVolatileStorageFixed.  Using this feature will make probing take
         * more time, as it means that the entire calibration data must be read, instead of just the
         * header.  The list of configs can include a mixture of devices with some using this, and
         * some not using it.
         *
         * The cache-on-disk feature of NonVolatileStorageShadow can also be used, this is activated
         * when cache-in-memory is enabled and in addition FlashMemoryConfig.useCaching is true.
         */
        class ModuleConfigFactoryByStorageIdBase : public IModuleConfigFactory
        {
        public:
            using value_type = royale::Pair<const royale::Vector<uint8_t>, const royale::config::ModuleConfig>;

            ROYALE_API ModuleConfigFactoryByStorageIdBase (
                const royale::Vector<value_type> configs,
                const royale::Vector<uint8_t> &defaultId = {});

            ROYALE_API royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> enumerateConfigs() const override;

            /**
             * Implementation of part of IModuleConfigFactory::probeAndCreate.  The returned
             * ModuleConfig is a copy of the original, and can be modified without modifying the
             * original.
             *
             * This method will not keep a reference to the INonVolatileStorage, so it can be safely
             * called when using NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME.
             *
             * If the original has FlashMemoryType::FIXED with a nullptr in
             * FlashMemoryConfig::nonVolatileStorageFixed, then a cached copy of the
             * INonVolatileStorage passed as the argument will be constructed.
             *
             * The cache-on-disk feature of NonVolatileStorageShadow can also be used by passing
             * cacheOnDisk=true, however it will only be used when the cache-in-memory is also
             * active.
             */
            ROYALE_API std::shared_ptr<royale::config::ModuleConfig> readAndCreate (royale::hal::INonVolatileStorage &storage,
                    bool cacheOnDisk) const;

            /**
             * If a defaultId was provided return the default config with its FlashMemoryType set
             * to FlashMemoryType::NONE. If there is no defaultId, this returns nullptr.
             *
             * If the user tries to use depth data this will fail at a later point, but if the user
             * only wants to use raw data then the calibration data is not needed.
             */
            ROYALE_API std::shared_ptr<royale::config::ModuleConfig> createWithoutStorage() const;

        private:
            /**
             * Search the m_moduleConfigs for this id, returns nullptr if it isn't found.
             */
            std::shared_ptr<royale::config::ModuleConfig> findConfig (const royale::Vector<uint8_t> &id) const;

            const royale::Vector<value_type> m_moduleConfigs;
            const royale::Vector<uint8_t> m_defaultId;
        };
    }
}
