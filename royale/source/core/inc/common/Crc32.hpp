/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

#include <royale/Definitions.hpp>

namespace royale
{
    namespace common
    {
        ROYALE_API uint32_t calculateCRC32 (const uint8_t *data, std::size_t len);
    }
}
