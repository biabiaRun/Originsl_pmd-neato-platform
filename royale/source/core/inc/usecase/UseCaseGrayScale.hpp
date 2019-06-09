/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Pair.hpp>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace usecase
    {
        class UseCaseGrayScale : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseGrayScale (royale::Pair<uint32_t, uint32_t> exposureLimits = { 1000, 1000 }, uint32_t exposureGray = 1000, bool expoOnForGray = false);
        };
    }
}
