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

#include <hal/ITemperatureSensor.hpp>
#include <common/ISensorRoutingConfig.hpp>
#include <config/TemperatureSensorConfig.hpp>
#include <usb/factory/IBridgeFactory.hpp>
#include <pal/II2cDeviceAccess.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        class TemperatureSensorFactory
        {
        public:
            static std::shared_ptr<royale::hal::ITemperatureSensor> createTemperatureSensor (
                royale::factory::IBridgeFactory &bridgeFactory,
                const royale::config::TemperatureSensorConfig &sensorConfig,
                const royale::common::ISensorRoutingConfig *sensorRouting);
        private:
            static std::shared_ptr<royale::hal::ITemperatureSensor> createADC1013TemperatureSensor (std::shared_ptr<royale::pal::II2cDeviceAccess> device,
                    const royale::config::TemperatureSensorConfig &sensorConfig);

        }; // class TemperatureSensorFactory

    } // namespace factory

} // namespace royale
