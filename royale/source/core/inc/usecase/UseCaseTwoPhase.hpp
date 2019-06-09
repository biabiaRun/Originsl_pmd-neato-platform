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

#include <royale/Pair.hpp>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace usecase
    {
        class UseCaseTwoPhase : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseTwoPhase (uint16_t frameRate,
                                        royale::Pair<uint32_t, uint32_t> exposureSettings,
                                        uint32_t exposureGray1,          // 0 ... disabled
                                        uint32_t exposureGray2,          // 0 ... disabled
                                        ExposureGray expoOnForGray1 = ExposureGray::Off,
                                        ExposureGray expoOnForGray2 = ExposureGray::Off);
        };
    }
}
