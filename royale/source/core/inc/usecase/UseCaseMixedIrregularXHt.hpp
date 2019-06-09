/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
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
        /**
         * A mixed-mode Use Case with an x:1 hand tracking to environment scanning ratio.  The x:1
         * ratio is given by the constructor's ratio argument; an exception is thrown if x is zero.
         *
         * The htRate used for this use case's constructor is the number of HT groups per second.
         * Therefore the environment scanning runs at (htRate/ratio) Hz.
         *
         * The target rate is (the number of HT groups + 2 * the number of ES groups) per second.
         * This meaning of the rate is used by getTargetRate(), setTargetRate() and getMaxRate().
         *
         * The frames are captured in the order HT1, HT2, ..., HTx, ES.  This is a non-interleaved
         * Use Case, which means that each HT or ES group is composed of consecutive frames.
         * The difference to UseCaseMixedXHt is, that the ES measurement doesn't have to fit in between
         * two HT measurements with equidistant spacing. The HT are moved forward in this case.
         *
         * The gray frames use the same exposure limits as the corresponding modulated ones.
         */
        class UseCaseMixedIrregularXHt : public UseCaseDefinition
        {
        public:
            /**
             * The htRate is the number of HT groups per second, the environment scanning runs
             * at (htRate/ratio) Hz.
             */
            ROYALE_API UseCaseMixedIrregularXHt (
                uint16_t htRate,
                uint16_t ratio,
                uint32_t modulationFrequencyHt,
                uint32_t modulationFrequencyEs1,
                uint32_t modulationFrequencyEs2,
                royale::Pair<uint32_t, uint32_t> exposureLimitsHt,
                royale::Pair<uint32_t, uint32_t> exposureLimitsEs,
                uint32_t exposureModulationHt,
                uint32_t exposureModulationEs1,
                uint32_t exposureModulationEs2,
                uint32_t exposureGrayHt,   // 0 ... disabled
                uint32_t exposureGrayEs,   // 0 ... disabled
                ExposureGray expoOnForGrayHt = ExposureGray::Off,
                ExposureGray expoOnForGrayEs = ExposureGray::Off,
                IntensityPhaseOrder intensityAsFirstPhase = IntensityPhaseOrder::IntensityLastPhase,
                bool enableSSC = false,
                double ssc_freq = 0.,
                double ssc_kspread = 0.,
                double ssc_delta_modHt = 0.,
                double ssc_delta_modEs1 = 0.,
                double ssc_delta_modEs2 = 0.);
        };
    }
}
