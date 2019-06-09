/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
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
        namespace M2453_B11
        {
            /**
            * Converts the pseudodata of the M2453 sensor.
            *
            * Please note that the M2453 pseudo data contains more information than
            * implemented by this pseudo data interpreter. This class only implements
            * read operations for pseudo data content that is used by Royale.
            */
            class PseudoDataInterpreterB11 : public royale::common::PseudoDataTwelveBitCalculator
            {
            public:
                ~PseudoDataInterpreterB11() = default;

                PseudoDataInterpreterB11 (bool usesInternalCurrentMonitor = false) :
                    m_usesInternalCurrentMonitor (usesInternalCurrentMonitor)
                {
                }

                PseudoDataInterpreterB11 *clone() override
                {
                    return new PseudoDataInterpreterB11 (*this);
                }

                uint16_t getFrameNumber (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [3];
                }

                uint16_t getReconfigIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [2];
                }

                uint16_t getFollowingFrameNumber (uint16_t base, uint16_t n) const override
                {
                    // The frame number of the M2453 only increases one per superframe
                    return static_cast<uint16_t> ( (base + 1) & 0x0FFF);
                }

                uint16_t getSequenceIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [5];
                }

                uint8_t getBinning (const royale::common::ICapturedRawFrame &frame) const override
                {
                    //imager has no binning module
                    return static_cast<uint8_t> (1);
                }

                uint16_t getHorizontalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (1u + frame.getPseudoData() [24] -
                                                  frame.getPseudoData() [23]);
                }

                uint16_t getVerticalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (1u + frame.getPseudoData() [26] -
                                                  frame.getPseudoData() [25]);
                }

                float getImagerTemperature (const royale::common::ICapturedRawFrame &frame) const override
                {
                    throw royale::common::NotImplemented ("Not supported");
                }

                void getTemperatureRawValues (const common::ICapturedRawFrame &frame,
                                              uint16_t &vRef1,
                                              uint16_t &vNtc1,
                                              uint16_t &vRef2,
                                              uint16_t &vNtc2,
                                              uint16_t &offset) const override
                {
                    offset = 0u;
                    //only use the relevant 12 Bits of the values
                    vRef1 = frame.getPseudoData() [vRef2V4] & 0xFFF;
                    vRef2 = frame.getPseudoData() [vRef2V1] & 0xFFF;
                    vNtc1 = frame.getPseudoData() [vNtc2V4] & 0xFFF;
                    vNtc2 = frame.getPseudoData() [vNtc2V1] & 0xFFF;
                }

                static const uint16_t vRef2V1 = 47u;
                static const uint16_t vRef2V4 = 48u;
                static const uint16_t vNtc2V1 = 49u;
                static const uint16_t vNtc2V4 = 50u;

                uint16_t getRequiredImageWidth() const override
                {
                    //this number needs to be reviewed on any update of the pseudodata interpreter
                    //54 is the highest index of the second NTC temperature measurement values
                    return 54u + 1u;
                }

                void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
                {
                    eyeError = 0u;

                    if (m_usesInternalCurrentMonitor)
                    {
                        eyeError |= frame.getPseudoData() [42] & 0xFFF;
                        eyeError |= (frame.getPseudoData() [43] & 0xF00) << 16;
                    }
                }

                bool supportsExposureFromPseudoData() const override
                {
                    return true;
                }

                uint32_t getExposureTime (const common::ICapturedRawFrame &frame, uint32_t modulationFrequency)  const override
                {
                    const std::array<uint16_t, 4> preScaleMap = { { 1, 8, 32, 128 } };
                    const uint16_t expoReg = static_cast<uint16_t> ( ( (frame.getPseudoData() [29] & 0xF) << 12) |
                                             (frame.getPseudoData() [28] & 0xFFF));
                    return (uint32_t) std::ceil (1000.0f * (float) preScaleMap[expoReg >> 14] *
                                                 (float) (expoReg & 0x3FFF) / ( (float) modulationFrequency / 1000.0f));
                }

            private:

                bool m_usesInternalCurrentMonitor;

            };
        }
    }
}
