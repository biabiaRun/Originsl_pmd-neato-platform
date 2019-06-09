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

#include <cstdint>

namespace royale
{
    namespace imager
    {
        //The M2450 AIO firmware and the integrating software imager introduce some limitations,
        //the worst case values of this header file can be used by module configs to define
        //valid use cases.
        namespace M2450_A12
        {
            /**
            * Minimum exposure for normal (non-mixed) mode to always give the imager enough time for
            * safe-reconfig.
            */
            static const uint16_t MIN_EXPO_NR = 27;

            /**
            * Minimum exposure for mixed mode to always give the AIO firmware enough time to safe-reconfig,
            * even in the worst-case (maximum number of changes).
            */
            static const uint16_t MIN_EXPO_MX = 134;
        }
    }
}
