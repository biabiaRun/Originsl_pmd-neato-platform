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

#include <hal/IPsdTemperatureSensor.hpp>
#include <hal/IImager.hpp>
#include <config/TemperatureSensorConfig.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        class PsdTemperatureSensorFactory
        {
        public:
            static std::unique_ptr<royale::hal::IPsdTemperatureSensor> createTemperatureSensor (
                royale::hal::IImager &imager,
                const royale::config::TemperatureSensorConfig &sensorConfig);

        }; // class PsdTemperatureSensorFactory

    } // namespace factory

} // namespace royale
