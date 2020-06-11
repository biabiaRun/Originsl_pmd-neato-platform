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

#include <imager/M2452/PseudoDataInterpreter.hpp>
#include "common/exceptions/InvalidValue.hpp"

namespace royale
{
    namespace imager
    {
        namespace M2452
        {
            /**
            * Converts the pseudodata of the M2452 sensor with AllInOne firmware.
            */
            class PseudoDataInterpreter_AIO : public M2452::PseudoDataInterpreter
            {
            public:
                ~PseudoDataInterpreter_AIO() = default;

                PseudoDataInterpreter_AIO *clone() override
                {
                    return new PseudoDataInterpreter_AIO (*this);
                }

                std::vector<uint16_t> getTemperatureRawValues (const common::ICapturedRawFrame &frame) const override
                {
                    std::vector<uint16_t> result(4);
                    uint16_t &vRef1 = result[0];
                    uint16_t &vRef2 = result[1];
                    uint16_t &vNtc1 = result[2];
                    uint16_t &vNtc2 = result[3];
                    vRef1 = frame.getPseudoData() [vRef1I];
                    vRef2 = frame.getPseudoData() [vRef2I];
                    vNtc1 = frame.getPseudoData() [vNtc1I];
                    vNtc2 = frame.getPseudoData() [vNtc2I];
                    discardAllSurplusBits (
                        vRef1,
                        vRef2,
                        vNtc1,
                        vNtc2);
                    checkValues (
                        vRef1,
                        vRef2,
                        vNtc1,
                        vNtc2);
                    return result;
                }

                uint16_t getSequenceIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    LOG (WARN) << "seq idx " << frame.getPseudoData() [163];
                    return frame.getPseudoData() [163];
                }

            private:

                void discardAllSurplusBits (uint16_t &vRef1,
                                            uint16_t &vRef2,
                                            uint16_t &vNtc1,
                                            uint16_t &vNtc2) const
                {
                    auto discardSurplusBits = [ = ] (uint16_t &value)
                    {
                        const uint16_t mask = uint16_t ( (1 << validBits) - 1);
                        value &= mask;
                    };

                    discardSurplusBits (vRef1);
                    discardSurplusBits (vNtc1);
                    discardSurplusBits (vRef2);
                    discardSurplusBits (vNtc2);
                }

                void checkValues (uint16_t &vRef1,
                                  uint16_t &vRef2,
                                  uint16_t &vNtc1,
                                  uint16_t &vNtc2) const
                {

                    if (vRef2 > vRef1)
                    {
                        std::stringstream dt;
                        dt << __FILE__ << ":" << __LINE__
                           << ": Invalid value vRef2 > vRef1."
                           << " vRef1 = " << vRef1
                           << " vRef2 = " << vRef2
                           << " vNtc1 = " << vNtc1
                           << " vNtc2 = " << vNtc2;
                        std::string du = "received invalid temperature values";
                        throw royale::common::InvalidValue (dt.str(), du);
                    }
                    if (vNtc2 > vNtc1)
                    {
                        std::stringstream dt;
                        dt << __FILE__ << ":" << __LINE__
                           << ": Invalid value in NtcTempAlgo m_vNtc2 > m_vNtc1."
                           << " vRef1 = " << vRef1
                           << " vRef2 = " << vRef2
                           << " vNtc1 = " << vNtc1
                           << " vNtc2 = " << vNtc2;
                        std::string du = "received invalid temperature values";
                        throw royale::common::InvalidValue (dt.str(), du);
                    }
                }

                static const uint16_t vRef1I = 166;
                static const uint16_t vRef2I = 164;
                static const uint16_t vNtc1I = 165;
                static const uint16_t vNtc2I = 167;
                static const unsigned int validBits = 12;

                uint16_t getRequiredImageWidth() const override
                {
                    // This PDI references hardcoded locations up to an offset of vNtc2I.
                    return vNtc2I + 1;
                }

            };
        }
    }
}
