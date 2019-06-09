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

#include <common/ICapturedRawFrame.hpp>

#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * An instance of this class translates the opaque array returned from
         * CapturedRawFrame::getPseudoData.
         *
         * Each instance of this class is valid for all frames returned from that
         * device.
         */
        class IPseudoDataInterpreter
        {
        public:
            virtual ~IPseudoDataInterpreter() = default;

            /**
            * Returns a new instance of the actual class type.
            */
            virtual IPseudoDataInterpreter *clone() = 0;

            /**
             * Wrap-round counter of how many frames the Imager has captured.  This is for
             * the Bridge to check that all RawFrames belong to the same UseCaseDefinition.
             * This value the Mira DevSpec calls "frame counter".
             *
             * \param   frame   Contains the data from one raw Frame of the imager.
             */
            virtual uint16_t getFrameNumber (const common::ICapturedRawFrame &frame) const = 0;

            /**
            * Returns the reconfiguration index which is provided by the safe-reconfiguration firmware
            *
            * \param   frame   Contains the data from one raw Frame of the imager.
            */
            virtual uint16_t getReconfigIndex (const common::ICapturedRawFrame &frame) const = 0;

            /**
             * Returns the expected frame number for the n-th frame after the base frame.
             *
             * The frame numbers wrap-round, this encapsulates the wrapping logic.
             */
            virtual uint16_t getFollowingFrameNumber (uint16_t base, uint16_t n) const = 0;

            /**
             * Returns true if the second argument is more than the first argument, using the
             * frame/reconfig numbers' wrapping logic. For example, if this imager uses 12-bit numbers,
             * then 4000 is greater than 3000, but 100 is greater than 4000. Either frame numbers
             * are compared or reconfiguration indices are compared, it makes no sense to compare
             * a frame number with a reconfiguration index.
             */
            virtual bool isGreaterFrame (uint16_t base, uint16_t n) const = 0;

            /**
             * Returns the number of frames that lhs is later than rhs, taking wrap-around into
             * account, i.e. lhs - rhs with modulo arithmetics.
             */
            virtual uint16_t frameNumberFwdDistance (uint16_t lhs, uint16_t rhs) const = 0;

            /**
             * Corresponds to the ImagerRawFrame's index in the ImagerUseCaseDefinition's
             * list ImagerUseCaseDefinition::m_rawFrames.
             *
             * \param   frame   Contains the data from one raw Frame of the imager.
             */
            virtual uint16_t getSequenceIndex (const common::ICapturedRawFrame &frame) const = 0;

            /**
             * The binning configuration.
             *
             * \param   frame   Contains the data from one raw Frame of the imager.
             */
            virtual uint8_t getBinning (const common::ICapturedRawFrame &frame) const = 0;

            /**
             * The horizontal size, including binning.
             *
             * \param   frame   Contains the data from one raw Frame of the imager.
             */
            virtual uint16_t getHorizontalSize (const common::ICapturedRawFrame &frame) const = 0;

            /**
             * The vertical size, including binning.
             *
             * \param   frame   Contains the data from one raw Frame of the imager.
             */
            virtual uint16_t getVerticalSize (const common::ICapturedRawFrame &frame) const = 0;

            /**
             * Calibrated result of the temperature measurement in degrees of celsius.
             * The iSM temperature measurement during pre-illumination phase must be activated.
             *
             * \param   frame   Contains the data from one raw Frame of the imager.
             */
            virtual float getImagerTemperature (const common::ICapturedRawFrame &frame) const = 0;

            /**
             * This function returns the raw ADC readings if supported by the concrete
             * implementation of IPseudoDataInterpreter.
             *
             * Some imagers include an ADC to read from an external peripheral, for example a thermistor,
             * and include the readings in the pseudodata. If this feature is supported and active,
             * this returns the raw temperature readings.
             *
             * \param   frame   Contains the data from one raw frame of the imager.
             * \param   vRef1   First pixel of the raw temperature reading.
             * \param   vNtc1   Second pixel of the raw temperature reading.
             * \param   vRef2   Third pixel of the raw temperature reading.
             * \param   vNtc2   Fourth pixel of the raw temperature reading.
             * \param   offset  Reserved.
             */
            virtual void getTemperatureRawValues (const common::ICapturedRawFrame &frame, uint16_t &vRef1, uint16_t &vNtc1, uint16_t &vRef2, uint16_t &vNtc2, uint16_t &offset) const = 0;

            /**
             * Returns the minimum required image width in pixels.
             *
             * This covers the data accessed by the PseudoDataInterpreter,
             * but doesn't take alignment requirements into account.
             * Typically imagers only allow image widths in certain multiples
             * (e.g. multiples of 16), so the value retuned here may be not
             * a valid width for the imager (in that case, it needs to be rounded
             * up by the caller).
             */
            virtual uint16_t getRequiredImageWidth() const = 0;

            /**
             * This function returns the eye safety monitor status if supported by the concrete
             * implementation of IPseudoDataInterpreter. If it is not supported eyeError will
             * always be zero.
             *
             * Some imagers include an internal current monitor for checking the eye safety.
             * If this monitor triggers, this function will return an eyeError different from zero and
             * the value can be used to distinguish the error. The error that is returned depends on the
             * concrete IPseudoDataInterpreter implementation. During normal operation eyeError will
             * always be zero.
             *
             * \param   frame      Contains the data from one raw frame of the imager.
             * \param   eyeError   Eye safety monitor status.
             */
            virtual void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const = 0;

            /**
             * This function checks if the imager that is used supports reading out the exposure time
             * of the current frame from the pseudodata.
             */
            virtual bool supportsExposureFromPseudoData() const = 0;

            /**
             * This function calculates the current exposure time based on the values given in the pseudodata and
             * the modulation frequency.
             */
            virtual uint32_t getExposureTime (const common::ICapturedRawFrame &frame, uint32_t modulationFrequency) const = 0;
        };
    }
}
