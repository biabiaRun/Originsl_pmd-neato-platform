/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <storage/SpiBusMasterM2453.hpp>

namespace royale
{
    namespace storage
    {
        /**
         * The SpiBusMasterM2455 is currently implemented as an alias to the SpiBusMasterM2453.
         *
         * For read-only functionality the behavior is the same, for writing the correct
         * ImagerAsBridgeType must be passed to SpiBusMasterM2453's constructor, as it enables a
         * small workaround.
         */
        using SpiBusMasterM2455 = SpiBusMasterM2453;
    }
}
