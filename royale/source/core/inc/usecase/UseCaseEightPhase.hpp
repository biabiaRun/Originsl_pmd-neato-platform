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

#include <royale/Definitions.hpp>
#include <royale/Pair.hpp>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace usecase
    {
        class UseCaseEightPhase : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseEightPhase (
                uint16_t targetFrameRate,
                uint32_t modulationFrequency1,
                uint32_t modulationFrequency2,
                royale::Pair<uint32_t, uint32_t> exposureLimits,
                uint32_t exposureModulation1,
                uint32_t exposureModulation2,
                uint32_t exposureGray,   // 0 ... disabled
                ExposureGray expoOnForGray = ExposureGray::Off,
                IntensityPhaseOrder intensityAsFirstPhase = IntensityPhaseOrder::IntensityLastPhase,
                bool enableSSC = false,
                double ssc_freq = 0.,
                double ssc_kspread = 0.,
                double ssc_delta_mod1 = 0.,
                double ssc_delta_mod2 = 0.);
        private:
            RawFrameSet createPhaseRFS (
                uint32_t modulationFrequency,
                ExposureGroupIdx exposureGroupIdx,
                double ssc_freq,
                double ssc_kspread,
                double ssc_delta);
        };
    }
}
