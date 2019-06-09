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

#include <royale/Definitions.hpp>

namespace royale
{
    namespace usb
    {
        namespace config
        {
            /**
             * Bridge type corresponds to the module firmware type.
             * There may be more than one bridge implementation for a particular
            * bridge type, e.g. for different operating systems or USB libraries.
             *
                   */
            enum class BridgeType
            {
                ENCLUSTRA  = 1,  //!< Enclustra firmware
                PMD_FPGA   = 2,  //!< PMDPlatform (not supported anymore)
                UVC        = 3,  //!< USB Video Class (plus extensions)
                AMUNDSEN   = 4,  //!< Similar to UVC, changed to avoid Win10's Frame Server
            };
        }
    }
}
