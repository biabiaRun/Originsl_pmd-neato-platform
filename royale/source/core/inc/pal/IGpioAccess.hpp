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

#include <cstdint>

namespace royale
{
    namespace pal
    {
        /**
         * This is the generic interface for supporting arbitrary GPIOs on the device
         */
        class IGpioAccess
        {
        public:
            enum class GpioState
            {
                LOW,
                HIGH,
                /** HighZ or input state. */
                Z
            };

            virtual ~IGpioAccess() = default;

            /**
             * Drive a GPIO high or low.  The GPIO ids are defined either by the class that
             * implements this function, or possibly by the hardware itself.
             *
             * The GPIO may also be set to high-impedence state with this function.
             */
            virtual void setGpio (uint32_t id, GpioState state) = 0;
        };
    }
}
