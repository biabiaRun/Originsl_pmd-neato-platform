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

#ifdef _WIN32
#    define IMAGER_EXPORT __declspec(dllexport)
#else
#    define IMAGER_EXPORT
#endif

namespace royale
{
    namespace imager
    {
        /**
        * All enums of the imager component shall start with a non-zero index
        * to uncover possible errors when directly casting from external enums.
        */
        static const uint16_t IMG_ENUM = 0x8000;
    }
}
