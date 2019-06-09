/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <pal/II2cDeviceAccess.hpp>
#include <hal/ITemperatureSensor.hpp>

#include <memory>

namespace royale
{
    namespace sensors
    {
        class TemperatureSensorTMP102 : public royale::hal::ITemperatureSensor
        {
        public:
            ROYALE_API explicit TemperatureSensorTMP102 (std::shared_ptr<pal::II2cDeviceAccess> bridge);
            ~TemperatureSensorTMP102() override;
            float getTemperature() override;

        private:
            std::shared_ptr<pal::II2cDeviceAccess> m_device;
        };
    }
}
