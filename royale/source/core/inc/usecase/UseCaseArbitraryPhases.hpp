/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
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
        enum class UseCaseArbitraryPhaseSettingType
        {
            FourPhase = 0,
            GrayScaleIlluminationOff = 1,
            GrayScaleIlluminationOn = 2,
        };
        struct UseCaseArbitraryPhaseSetting
        {
            UseCaseArbitraryPhaseSettingType phaseSettingType = UseCaseArbitraryPhaseSettingType::FourPhase;
            uint32_t modulationFrequency;
            Pair<uint32_t, uint32_t> exposureLimits;
            uint32_t exposureTime;
            double ssc_freq;
            double ssc_kspread;
            double ssc_delta;
            UseCaseArbitraryPhaseSetting (
                UseCaseArbitraryPhaseSettingType phaseSettingType = UseCaseArbitraryPhaseSettingType::FourPhase,
                uint32_t modulationFrequency = 30000000u,
                royale::Pair<uint32_t, uint32_t> exposureLimits = royale::Pair<uint32_t, uint32_t> {1u, 1000u},
                uint32_t exposureTime = 100u,
                double ssc_freq = 0.0,
                double ssc_kspread = 0.0,
                double ssc_delta = 0.0) :
                phaseSettingType (phaseSettingType),
                modulationFrequency (modulationFrequency),
                exposureLimits (exposureLimits),
                exposureTime (exposureTime),
                ssc_freq (ssc_freq),
                ssc_kspread (ssc_kspread),
                ssc_delta (ssc_delta)
            {};
        };

        /***
         * Helper class to construct use case definitions with arbitrary combinations
         * of grayscale and modulated four phase measurements in one stream.
         * The use case is given by a concatenation of multiple
         * UseCaseArbitraryPhaseSetting objects.
         */

        class UseCaseArbitraryPhases : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseArbitraryPhases (
                uint16_t targetFrameRate,
                const UseCaseIdentifier &identifier,
                const royale::Vector<UseCaseArbitraryPhaseSetting> &Settings,
                bool enableSSC = false);
            ROYALE_API UseCaseArbitraryPhases (
                uint16_t targetFrameRate,
                const royale::Vector<UseCaseArbitraryPhaseSetting> &Settings,
                bool enableSSC = false);
        private:
            RawFrameSet createPhaseRFS (
                uint32_t modulationFrequency,
                ExposureGroupIdx exposureGroupIdx,
                double ssc_freq,
                double ssc_kspread,
                double ssc_delta);
            RawFrameSet createGrayRFS_SSC (
                uint32_t modulationFrequency,
                ExposureGroupIdx exposureGroupIdx,
                ExposureGray expoOn,
                double ssc_freq,
                double ssc_kspread,
                double ssc_delta);
        };
    }
}
