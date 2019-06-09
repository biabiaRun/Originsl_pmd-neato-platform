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
        class UseCaseCalibration : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseCalibration (uint16_t frameRate,
                                           uint32_t modulationFrequency1,
                                           uint32_t modulationFrequency2, // will also be used for the illuminated gray phase
                                           royale::Pair<uint32_t, uint32_t> exposureSettings,
                                           uint32_t exposureModulation1,
                                           uint32_t exposureModulation2,
                                           uint32_t exposureGray1,  // should be > 0
                                           uint32_t exposureGray2,  // should be > 0
                                           bool enableSSC = false,
                                           double ssc_freq_mod1 = 0.,
                                           double ssc_kspread_mod1 = 0.,
                                           double ssc_delta_mod1 = 0.,
                                           double ssc_freq_mod2 = 0.,
                                           double ssc_kspread_mod2 = 0.,
                                           double ssc_delta_mod2 = 0.);
        };
    }
}
