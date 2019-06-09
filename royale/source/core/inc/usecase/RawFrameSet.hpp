/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <usecase/ExposureGroup.hpp>
#include <vector>
#include <cstdint>

namespace royale
{
    namespace usecase
    {
        /**
         * Each RawFrameSet contains one or more raw frames which share the same settings.
         */
        class RawFrameSet
        {
        public:
            ROYALE_API static uint32_t MODFREQ_AUTO();

            enum class PhaseDefinition
            {
                /**
                 * This RFS contains one raw frame, an unmodulated one
                 */
                GRAYSCALE = 1,
                /**
                 * This RFS contains four raw frames
                 */
                MODULATED_4PH_CW = 0
            };

            enum class DutyCycle
            {
                DC_AUTO, //!< Value for dutyCycle to let the imager use the dutycycle configuration from the module config.
                DC_0,
                DC_25,
                DC_25_DEPRECATED, //!< 25% trailing dutycycle for legacy support of picoFlexx calibration only
                DC_37_5,
                DC_37_5_DEPRECATED, //!< 37.5% trailing dutycycle for a specific customer project
                DC_50,
                DC_75,
                DC_100
            };

            /***
             * Defines how the raw frame set will be positioned in the measurement sequence.
             */
            enum class Alignment
            {
                /**
                 * Raw frame sets having CLOCK_ALIGNED are spread evenly over the measurement
                 * sequence.
                 *
                 * In case of a non-mixed usecase, this alignment would only be used on the first
                 * raw frame set in the sequence.
                 *
                 * For mixed-mode usecases, it is often possible to designate one stream as "master"
                 * to which all other streams are aligned (this is the case when the least common
                 * multiple of all stream ratios is equal to the ratio of the master).  In that case
                 * CLOCK_ALIGNED would be set for one (e.g. the first) raw frame set in each frame
                 * group of the master stream, and all other raw frame sets would be aligned
                 * relative to these.
                 *
                 * If no "master" stream can be designated, a possible workaround would be to define
                 * a "pseudo" stream with the appropriate ratio; these would consist of raw frame
                 * sets which don't actually produce raw frames, but are just there for alignment.
                 */
                CLOCK_ALIGNED,

                /**
                 * The raw frame set is aligned to start immediately after the previous raw
                 * frame set in the measurement sequence.
                 */
                START_ALIGNED,

                /**
                 * The raw frame set is aligned to end immediately before the next raw frame set in
                 * the sequence.
                 */
                STOP_ALIGNED,

                /**
                 * The raw frame set is aligned to start immediately after a "virtual" clock aligned
                 * measurement which is inserted. So it is placed at the end of the next block.
                 */
                NEXTSTOP_ALIGNED,
            };

            RawFrameSet();
            RawFrameSet (uint32_t freq, PhaseDefinition phases, DutyCycle cycle, ExposureGroupIdx egroup,
                         Alignment align = Alignment::START_ALIGNED, double tEye = 0.,
                         double ssc_freq = 0., double ssc_kspread = 0., double ssc_delta = 0);

            bool operator== (const RawFrameSet &rhs) const;
            bool operator!= (const RawFrameSet &rhs) const;

            bool isModulated() const;
            bool isGrayscale() const;
            std::size_t countRawFrames() const;
            const std::vector<uint16_t> &getPhaseAngles() const;

            uint32_t              modulationFrequency;   //!< Frequency of the modulation PLL in [Hz]
            double                ssc_freq;              //!< the frequency of the SSC modulation
            double                ssc_kspread;           //!< the deviation of the actual frequency around the nominal modulation frequency
            double                ssc_delta;             //!< the total frequency deviation used during SSC modulation
            PhaseDefinition       phaseDefinition;       //!< indicates if the raw frames are modulated or gray scale only
            DutyCycle             dutyCycle;             //!< illumination signal dutycycle configuration
            ExposureGroupIdx      exposureGroupIdx;      //!< Index of the exposure group this frameset belongs to.
            Alignment             alignment;             //!< Indicates how the raw frame set is to be aligned in the measurement sequence.

            /**
             * The minimum guaranted time gap between the end of the last exposure of this set of
             * raw frames and the start of the first exposure of the next raw frame set.
             */
            double                tEyeSafety;

        private:
            /**
             * Value for modulationFrequency to let the imager automatically choose a frequency.
             */
            static const uint32_t M_MODFREQ_AUTO;
        };
    }
}
