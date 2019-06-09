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
#include <config/IlluminationPad.hpp>
#include <usecase/RawFrameSet.hpp>

#include <cstdint>

namespace royale
{
    namespace config
    {
        /**
         * Data container for illumination centric parameters
         */
        struct ROYALE_API IlluminationConfig
        {
            /**
            * The duty cycle of illumination signal this illumination circuit is expecting.
            */
            usecase::RawFrameSet::DutyCycle dutyCycle;

            /**
            * Hardware limit, measured in Hertz.
            */
            uint32_t maxModulationFrequency;

            /**
            * The imager's pad the illumination unit is connected to.
            */
            IlluminationPad illuminationPad;
        };
    }
}
