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

namespace royale
{
    namespace config
    {
        /**
        * Defines the different types of imagers which are available for creating a module
        */
        enum class ImagerType
        {
            M2450_A11,      // M2450 A11 (deprecated, support for picoS camera only)
            M2450_A12_AIO,  // M2450 A12 (using AllInOne RAM firmware)
            M2452_B1x_AIO,  // M2452 B1x AIO (using AllInOne firmware)
            M2453_A11,      // M2453 A11 (external configuration based)
            M2453_B11,      // M2453 B11 (external configuration based)
            M2455_A11,      // M2455 A11 (external configuration based)
            MXXXX_DUMMY     // A placeholder for some future Mxxxx imager
        };
    }
}
