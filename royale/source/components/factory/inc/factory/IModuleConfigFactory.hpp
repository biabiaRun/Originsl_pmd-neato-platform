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

#include <usb/factory/IBridgeFactory.hpp>
#include <config/ModuleConfig.hpp>
#include <royale/Vector.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        /**
         * Factory which creates a ModuleConfig.
         *
         * This factory is used during device enumeration to determine the configuration
         * of the discovered device. An instance of this is kept in UsbProbeData for every
         * supported VID/PID and bridge type.
         *
         * In most cases the VID/PID corresponds to exactly one ModuleConfig (with all
         * details known beforehand); for these a simple implementation (which just returns
         * a fixed module config) is sufficient.
         *
         * If VID/PIDs are shared for different modules (as with the Skylla boards), or
         * if a module can be fitted with different components which need different
         * configurations (as with devboards), the factory may need to access the hardware
         * to determine the actual configuration.
         * As there is one instance per VID/PID (and bridge type), different probing
         * algorithms can be implemented for different device families.
         *
         * \see ModuleConfigFactoryFixed for a trivial implementation.
         */
        class IModuleConfigFactory
        {
        public:
            virtual ~IModuleConfigFactory() = default;

            /**
             * Create a ModuleConfig for the device associated with a bridge factory.
             *
             * Given an initialized bridge factory, the factory method should
             * return a ModuleConfig that corresponds to the connected hardware.
             *
             * Implementations are allowd to instantiate bridge interfaces in
             * the bridge factory and access the hardware if necessary.
             *
             * \param bridgeFactory The bridge factory for the device, already initialized.
             * \return A complete ModuleConfig, or nullptr if it cannot be determined.
             */
            virtual std::shared_ptr<const royale::config::ModuleConfig>
            probeAndCreate (royale::factory::IBridgeFactory &bridgeFactory) const = 0;

            /**
             * Return a possibly-incomplete list of all ModuleConfigs that could be returned by
             * probeAndCreate.  This is intended for sanity checks such as
             * UnitTestModuleConfig.AllModulesCoreConfigFactory
             *
             * This method does not access the device, it merely returns the ModuleConfigs that the
             * factory could return.  The list may be incomplete, for example a factory that
             * generates the config's use cases from data in the device's storage may return an
             * empty list.
             */
            virtual royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> enumerateConfigs() const = 0;
        };
    }
}
