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

#include <imager/ImagerCommon.hpp>

#include <cstdint>

namespace royale
{
    namespace imager
    {
        /**
        * A raw frame is the smallest configurable unit of the imager
        */
        class ImagerRawFrame
        {
        public:
            IMAGER_EXPORT static uint32_t MODFREQ_AUTO();

            enum class ImagerDutyCycle
            {
                DC_AUTO = IMG_ENUM, //!< Value for dutyCycle to let the imager use the dutycycle configuration from the ImagerParameters.
                DC_0,
                DC_25,
                DC_25_DEPRECATED, //!< 25% trailing dutycycle for legacy support of picoFlexx calibration only
                DC_37_5,
                DC_37_5_DEPRECATED, //!< 37.5% trailing dutycycle for a specific customer project
                DC_50,
                DC_75,
                DC_100
            };

            /**
            * Defines how the raw frames will be positioned in the measurement sequence.
            */
            enum class ImagerAlignment
            {
                /**
                * Raw frames having CLOCK_ALIGNED are spread evenly over the measurement
                * sequence.
                */
                CLOCK_ALIGNED = IMG_ENUM,

                /**
                * The raw frame is aligned to start immediately after the previous raw
                * frame in the measurement sequence.
                */
                START_ALIGNED,

                /**
                * The raw frame is aligned to end immediately before the next raw frame in
                * the sequence.
                */
                STOP_ALIGNED,

                /**
                * The raw frame is aligned to start immediately after a "virtual" clock aligned
                * measurement which is inserted. So it is placed at the end of the next block.
                */
                NEXTSTOP_ALIGNED,
            };

            IMAGER_EXPORT ImagerRawFrame();
            IMAGER_EXPORT ImagerRawFrame (uint32_t freq, bool grayscale, bool isStartOfLinkedRawFrames, bool isEndOfLinkedRawFrames,
                                          bool isEndOfLinkedMeasurement, ImagerDutyCycle cycle, uint32_t expoTime, uint16_t phaseAngle,
                                          ImagerAlignment align = ImagerAlignment::START_ALIGNED, double tEye = 0.,
                                          double ssc_freq = 0., double ssc_kspread = 0., double ssc_delta = 0);

            bool operator== (const ImagerRawFrame &rhs) const;
            bool operator!= (const ImagerRawFrame &rhs) const;

            uint32_t              modulationFrequency; //!< Frequency of the modulation PLL in [Hz].
            double                ssc_freq;            //!< the frequency of the SSC modulation.
            double                ssc_kspread;         //!< the deviation of the actual frequency around the nominal modulation frequency.
            double                ssc_delta;           //!< the total frequency deviation used during SSC modulation.
            bool                  grayscale;           //!< indicates if the raw frames are modulated or gray scale only.
            ImagerDutyCycle       dutyCycle;           //!< illumination signal dutycycle configuration.
            uint16_t              phaseAngle;          //!< phase angle of the raw frane.
            uint32_t              exposureTime;        //!< Exposure time of the raw frame.
            ImagerAlignment       alignment;           //!< Indicates how the raw frame set is to be aligned in the measurement sequence.

            /**
            * Indicates if the raw frame is the first of a set of adjunctive raw frames.
            * E.g. it is the first raw frame of a 4-phase measurement.
            */
            bool   isStartOfLinkedRawFrames;

            /**
            * Indicates if the raw frame is the last of a set of adjunctive raw frames.
            * E.g. it is the last raw frame of a 4-phase measurement.
            */
            bool   isEndOfLinkedRawFrames;

            /**
            * Indicates if the raw frame is the last of a complete measurement.
            * E.g.it is the last frame of two 4-phase measurements which belong together for resolving
            * the unambiguity problem - so the complete measurement takes 8 raw frames, the last raw frame is
            * marked by this flag and the fourth and the last raw frame is marked using isEndOfLinkedRawFrames.
            * Please note that the imager will only reconfigure (by calling IImagerComponent::reconfigure)
            * such a complete measurement at once.
            */
            bool   isEndOfLinkedMeasurement;

            /**
            * The minimum guaranted time gap between the end of the last exposure of this
            * raw frame and the start of the first exposure of the next raw frame.
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
