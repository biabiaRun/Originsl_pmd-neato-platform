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

#include <common/exceptions/RuntimeError.hpp>

namespace royale
{
    namespace common
    {
        /**
         * Indicates that the device may have detected and signalled an error by sending a
         * USB STALL.  The higher-layer protocol may be able to query the firmware for more
         * details about the error.
         *
         * This may also indicate an error somewhere else in the USB stack, there may be no
         * information retrievable from the firmware.
         */
        class PossiblyUsbStallError : public royale::common::RuntimeError
        {
        public:
            PossiblyUsbStallError() :
                RuntimeError ("USB I/O error, possibly USB STALL")
            {
            }

            ~PossiblyUsbStallError() = default;
        };
    }
}
