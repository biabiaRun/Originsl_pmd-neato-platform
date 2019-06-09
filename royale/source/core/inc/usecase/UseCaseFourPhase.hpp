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

#include <usecase/UseCaseDefinition.hpp>
#include <royale/Pair.hpp>

namespace royale
{
    namespace usecase
    {
        class UseCaseFourPhase : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseFourPhase (
                uint16_t targetFrameRate,
                uint32_t modulationFrequency,
                royale::Pair<uint32_t, uint32_t> exposureLimits,
                uint32_t exposureModulation,
                uint32_t exposureGray, // 0 ... disabled
                ExposureGray expoOnForGray = ExposureGray::Off,
                IntensityPhaseOrder intensityAsFirstPhase = IntensityPhaseOrder::IntensityLastPhase,
                bool enableSSC = false,
                double ssc_freq = 0.,
                double ssc_kspread = 0.,
                double ssc_delta = 0.);
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
