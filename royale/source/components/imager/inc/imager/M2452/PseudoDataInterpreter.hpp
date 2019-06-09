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
#include <array>
#include <cmath>

namespace royale
{
    namespace imager
    {
        namespace M2452
        {
            /**
            * Converts the pseudodata of the MiraLotte sensor.
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
                    return frame.getPseudoData() [14];
                }

                uint16_t getExposure (const royale::common::ICapturedRawFrame &frame)
                {
                    return static_cast<uint16_t> ( (frame.getPseudoData() [7] & 0xFF) << 8 | (frame.getPseudoData() [6] & 0xFF));
                }

                uint16_t getSequenceIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [1];
                }

                uint8_t getBinning (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint8_t> (1);
                }

                uint16_t getHorizontalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (frame.getPseudoData() [2] & 0xFF);
                }

                uint16_t getVerticalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (frame.getPseudoData() [3] & 0xFF);
                }

                float getImagerTemperature (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return 0;
                }

                uint32_t getExposureTime (const royale::common::ICapturedRawFrame &frame, uint32_t modfreq)
                {
                    const std::array<uint16_t, 4> preScaleMap = { { 1, 8, 32, 128 } };
                    const uint16_t expoReg = static_cast<uint16_t> ( ( (frame.getPseudoData() [7] & 0xFF) << 8) |
                                             (frame.getPseudoData() [6] & 0xFF));
                    return (uint32_t) std::ceil (1000.0f * (float) preScaleMap[expoReg >> 14] *
                                                 (float) (expoReg & 0x3FFF) / ( (float) modfreq / 1000.0f));
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
                    // This PDI references hardcoded locations up to an offset of 14.
                    return 14 + 1;
                }

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
