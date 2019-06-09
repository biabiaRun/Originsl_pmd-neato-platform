/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
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
         * An interleaved-mode Use Case with an x:1 hand tracking to environment scanning ratio.
         * The x:1 ratio is given by the constructor's ratio argument; an exception is thrown if x
         * is less than 2.
         *
         * The targetRate for this use case is the number of HT groups per second.  Therefore the
         * environment scanning runs at (targetRate/ratio) Hz.
         *
         * The frames are captured in the order HT1, ES1, HT2, ES2, HT3, ..., HTx. The ES1 and ES2
         * are part of a single ES group, this is an interleaved use case because HT2 is captured in
         * the middle of capturing ES.
         *
         * The gray frames use the same exposure limits as the corresponding modulated ones.
         */
        class UseCaseInterleavedXHt : public UseCaseDefinition
        {
        public:
            /**
             * The targetRate is the number of HT groups per second, the environment scanning runs
             * at (targetRate/ratio) Hz.
             */
            ROYALE_API UseCaseInterleavedXHt (
                uint16_t targetRate,
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
