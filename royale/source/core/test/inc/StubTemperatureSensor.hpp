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

#include <hal/ITemperatureSensor.hpp>

namespace royale
{
    namespace stub
    {
        namespace hal
        {
            class StubTemperatureSensor : public royale::hal::ITemperatureSensor
            {
            public:
                StubTemperatureSensor() = default;
                ~StubTemperatureSensor() = default;

                float getTemperature() override
                {
                    return m_temperature;
                }

                void setTemperature (float temperature)
                {
                    m_temperature = temperature;
                }

            private:
                float m_temperature = 20.0f;
            };
        }
    }
}
