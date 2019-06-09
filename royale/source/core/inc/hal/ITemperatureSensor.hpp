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

#include <royale/Definitions.hpp>

namespace royale
{
    namespace hal
    {
        class ITemperatureSensor
        {
        public:
            virtual ~ITemperatureSensor() = default;

            /**
             * The measurement is in Celsius.
             */
            virtual float getTemperature() = 0;
        };
    }
}
