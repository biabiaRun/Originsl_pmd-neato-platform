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
        * Defines the frame transmission mode used by the imager.
        * For the M2452 imager(s), this can be configured as SUPERFRAME or
        * INDIVIDUAL via the CFGCNT_CSICFG register. The M2450 imagers use
        * INDIVIDUAL.
        *
        */
        enum class FrameTransmissionMode
        {
            /**
            * Imager sends frame data followed by an end-of-frame marker for
            * each raw frame that is part of the frame group.
            */
            INDIVIDUAL,
            /**
            * Imager sends frame data for all raw frames followed by an end-of-frame marker
            * for the complete frame group.
            */
            SUPERFRAME
        };

    }
}
