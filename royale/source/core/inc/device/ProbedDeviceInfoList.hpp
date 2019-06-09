/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <device/ProbedDeviceInfo.hpp>
#include <royale/Vector.hpp>

namespace royale
{
    namespace device
    {
        /**
         * This class contains infos about devices which were probed.
         */
        using ProbedDeviceInfoList = royale::Vector<ProbedDeviceInfo>;
    }
}
