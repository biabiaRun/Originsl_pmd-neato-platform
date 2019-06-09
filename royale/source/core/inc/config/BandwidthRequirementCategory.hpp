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
        * Bandwidth requirement category used in determining raw framerate limits.
        */
        enum class BandwidthRequirementCategory
        {
            /**
            * No framerate limitations needed for USB2 or USB3.
            */
            NO_THROTTLING,
            /**
            * No framerate limitations needed for USB3, USB2 needs throttling.
            */
            USB2_THROTTLING,
            /**
            * USB3 needs framerate limits, module won't work with USB2.
            */
            USB3_THROTTLING
        };

    }
}

