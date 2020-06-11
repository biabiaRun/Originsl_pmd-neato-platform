/****************************************************************************\
 * Copyright (C) 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <imager/M2457/PseudoDataInterpreter.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <cmath>
#include <array>
#include <vector>

namespace royale
{
    namespace imager
    {
        namespace M2457
        {
            /**
             * Converts the pseudodata of the M2457 sensor with illumination IC.
             *
             * Please note that the M2457 pseudo data contains more information than
             * implemented by this pseudo data interpreter. This class only implements
             * read operations for pseudo data content that is used by Royale.
             */
            class PseudoDataInterpreter_IC
                : public M2457::PseudoDataInterpreter
            {
            public:
                ~PseudoDataInterpreter_IC() = default;

                PseudoDataInterpreter_IC (bool usesInternalCurrentMonitor = false)
                    : M2457::PseudoDataInterpreter (usesInternalCurrentMonitor) {}

                PseudoDataInterpreter_IC *clone() override
                {
                    return new PseudoDataInterpreter_IC (*this);
                }

                std::vector<uint16_t> getTemperatureRawValues (const common::ICapturedRawFrame &frame) const override
                {
                    std::vector<uint16_t> result (1);
                    uint16_t temp1 = static_cast<uint16_t> ( ( (frame.getPseudoData() [vIcTemp1]) & 0x0003u) << 8);
                    // only use the relevant 12 Bits of the values
                    uint16_t temp2 = frame.getPseudoData() [vIcTemp2];
                    result[0] = static_cast<uint16_t> (temp1 + temp2);
                    return result;
                }

                static const uint16_t vIcTemp1 = 199u;
                static const uint16_t vIcTemp2 = 200u;

                uint16_t getRequiredImageWidth() const override
                {
                    // this number needs to be reviewed on any update of the pseudodata
                    // interpreter
                    return 200u + 1u;
                }
            };
        } // namespace M2457
    } // namespace imager
} // namespace royale
