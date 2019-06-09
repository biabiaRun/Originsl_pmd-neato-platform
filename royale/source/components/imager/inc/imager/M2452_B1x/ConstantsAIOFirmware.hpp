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

namespace royale
{
    namespace imager
    {
        //The M2452 B1x AIO firmware and the integrating software imager introduce some limitations,
        //the worst case values of this header file can be used by module configs to define
        //valid use cases.
        namespace M2452_B1x
        {
            /**
            * Minimum exposure for normal (non-mixed) mode to always give the imager enough time for
            * safe-reconfig.
            */
            static const uint16_t MIN_EXPO_NR = 8u;
            /**
            * Minimum exposure for normal (non-mixed) mode if the NTC feature is enabled to always
            * give the imager enough time for safe-reconfig.
            */
            static const uint16_t MIN_EXPO_NR_WITH_NTC = 75u;
            /**
            * Minimum exposure for mixed mode to always give the AIO firmware enough time to safe-reconfig,
            * even in the worst-case (maximum number of changes).
            */
            static const uint16_t MIN_EXPO_MX = 133;
            /**
            * Minimum exposure for mixed mode if the NTC feature is enabled to always give the
            * AIO firmware enough time to safe-reconfig, even in the worst-case (maximum number of changes).
            */
            static const uint16_t MIN_EXPO_MX_WITH_NTC = 200;
        }
    }
}
