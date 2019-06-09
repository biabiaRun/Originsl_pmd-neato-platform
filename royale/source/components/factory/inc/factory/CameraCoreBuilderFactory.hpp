/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <factory/ICameraCoreBuilder.hpp>
#include <usb/config/BridgeType.hpp>
#include <config/ModuleConfig.hpp>

namespace royale
{
    namespace factory
    {
        // Factory for camera core builders (i.e. a factory that creates factories)
        class CameraCoreBuilderFactory
        {
        public:
            CameraCoreBuilderFactory() = default;
            CameraCoreBuilderFactory (royale::usb::config::BridgeType type,
                                      const royale::config::ModuleConfig &config);
            std::unique_ptr<royale::factory::ICameraCoreBuilder> operator() ();
        private:
            royale::usb::config::BridgeType     m_type;
            const royale::config::ModuleConfig &m_config;
        };
    }
}
