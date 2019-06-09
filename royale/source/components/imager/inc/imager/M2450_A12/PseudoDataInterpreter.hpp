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

#include <common/PseudoDataTwelveBitCalculator.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <algorithm>

namespace royale
{
    namespace imager
    {
        namespace M2450_A12
        {
            /**
            * Converts the pseudodata of the M2450 A12 sensor.
            */
            class PseudoDataInterpreter : public royale::common::PseudoDataTwelveBitCalculator
            {
            public:
                ~PseudoDataInterpreter() = default;

                PseudoDataInterpreter *clone() override
                {
                    return new PseudoDataInterpreter (*this);
                }

                uint16_t getFrameNumber (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [0];
                }

                uint16_t getReconfigIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [RECONFIG_INDEX];
                }

                uint16_t getSequenceIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (frame.getPseudoData() [1] >> 7);
                }

                uint8_t getBinning (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint8_t> ( (frame.getPseudoData() [1] >> 5) & 3);
                }

                uint16_t getHorizontalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> ( (frame.getPseudoData() [1] & 31) << 4 / (1 << getBinning (frame)));
                }

                uint16_t getVerticalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> ( (frame.getPseudoData() [2] & 511) / (1 << getBinning (frame)));
                }

                float getImagerTemperature (const royale::common::ICapturedRawFrame &frame) const override
                {
                    /* Errata Temperature Sensor:
                     * A11 RAM FW has to be loaded to get correct Temp.values
                     * A12 Moved to ROM
                     */

                    /* calculation defined in devspec:
                     * Temp = ADC_value * k + d
                     * k = (-273.15 - T_cal) / (2048 - ADC_expected - Calib * 4)
                     * d = -273.15 - 2048 * k
                     * Assumption : T_cal = 85 deg, ADC_expected = 3553 for LOTTE, 3447 for MiraCE_A12
                     */

                    // 1devspec says that the calib.value without division is between 60 and 76
                    // it says also that the value has to be multiplied by 4, but because
                    // the efuse&pseudodata gives the value of ~65 directly we discard the multiplication

                    //do not forget to enable the measurement by setting ISMSTATE (0x2 is enable)
                    const float adcRes = frame.getPseudoData() [5]; //odd value
                    const float calib = frame.getPseudoData() [7]; //calib (~60-76)

                    return calculateTemperature (adcRes, calib);
                }

                static float calculateTemperature (const float &adcValue, const float &calibrationValue)
                {
                    const float k = (-237.15f - 85.0f) / (2048.0f - 3447.0f - calibrationValue);
                    const float d = -237.15f - 2048.0f * k;

                    return adcValue * k + d;
                }

                void getTemperatureRawValues (const common::ICapturedRawFrame &frame,
                                              uint16_t &vRef1,
                                              uint16_t &vNtc1,
                                              uint16_t &vRef2,
                                              uint16_t &vNtc2,
                                              uint16_t &offset) const override
                {
                    throw royale::common::NotImplemented ("Not supported");
                }

                uint16_t getRequiredImageWidth() const override
                {
                    // This PDI references hardcoded locations up to an offset of 7
                    // as well as RECONFIG_INDEX, which is expected to be larger (but who knows).
                    // note: the "+0" is a workaround for a gcc6 issue (which causes a linker error)
                    return static_cast<uint16_t> (1 + std::max (7, RECONFIG_INDEX + 0));
                }

                static const uint16_t RECONFIG_INDEX = 148u;

                void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
                {
                    eyeError = 0u;
                }

                bool supportsExposureFromPseudoData() const override
                {
                    return false;
                }

                uint32_t getExposureTime (const common::ICapturedRawFrame &frame, uint32_t modulationFrequency)  const override
                {
                    throw royale::common::NotImplemented();
                }
            };
        }
    }
}
