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

#include <common/PseudoDataTwelveBitCalculator.hpp>
#include <common/exceptions/NotImplemented.hpp>

namespace royale
{
    namespace imager
    {
        namespace M2453_A11
        {
            /**
            * Converts the pseudodata of the M2453 sensor.
            *
            * Please note that the M2453 pseudo data contains more information than
            * implemented by this pseudo data interpreter. This class only implements
            * read operations for pseudo data content that is used by Royale.
            */
            class PseudoDataInterpreter : public royale::common::PseudoDataTwelveBitCalculator
            {
            public:
                ~PseudoDataInterpreter() = default;

                PseudoDataInterpreter (bool usesInternalCurrentMonitor = false) :
                    m_usesInternalCurrentMonitor (usesInternalCurrentMonitor)
                {
                }

                PseudoDataInterpreter *clone() override
                {
                    return new PseudoDataInterpreter (*this);
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
                    return frame.getPseudoData() [4];
                }

                uint8_t getBinning (const royale::common::ICapturedRawFrame &frame) const override
                {
                    //imager has no binning module
                    return static_cast<uint8_t> (1);
                }

                uint16_t getHorizontalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (1u + frame.getPseudoData() [23] -
                                                  frame.getPseudoData() [22]);
                }

                uint16_t getVerticalSize (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return static_cast<uint16_t> (1u + frame.getPseudoData() [25] -
                                                  frame.getPseudoData() [24]);
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

                static const uint16_t vRef2V1 = 46u;
                static const uint16_t vRef2V4 = 47u;
                static const uint16_t vNtc2V1 = 48u;
                static const uint16_t vNtc2V4 = 49u;

                uint16_t getRequiredImageWidth() const override
                {
                    //this number needs to be reviewed on any update of the pseudodata interpreter
                    return vNtc2V1 + 1u;
                }

                void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
                {
                    eyeError = 0u;

                    if (m_usesInternalCurrentMonitor)
                    {
                        eyeError |= frame.getPseudoData() [41] & 0x3FF;
                        eyeError |= (frame.getPseudoData() [42] & 0x1F) << 16;
                    }
                }

                bool supportsExposureFromPseudoData() const override
                {
                    return false;
                }

                uint32_t getExposureTime (const common::ICapturedRawFrame &frame, uint32_t modulationFrequency)  const override
                {
                    throw royale::common::NotImplemented();
                }

            private:

                bool m_usesInternalCurrentMonitor;

            };
        }
    }
}
