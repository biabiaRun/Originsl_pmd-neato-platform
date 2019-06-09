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

#include <imager/M2450_A12/PseudoDataInterpreter.hpp>

namespace royale
{
    namespace imager
    {
        namespace M2450_A12
        {
            /**
            * Converts the pseudodata of the M2450 A12 sensor with AIO firmware.
            * The different counter/index pixels were introducted due to the added
            * mixed mode support. However, the same pixels can be used if mixed
            * mode is disabled (this class is valid for all configurations of the
            * AIO firmware).
            */
            class PseudoDataInterpreter_AIO : public PseudoDataInterpreter
            {
            public:
                ~PseudoDataInterpreter_AIO() = default;

                PseudoDataInterpreter_AIO *clone() override
                {
                    return new PseudoDataInterpreter_AIO (*this);
                }

                uint16_t getFrameNumber (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [METAFRAMECNTR];
                }

                uint16_t getSequenceIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    return frame.getPseudoData() [METASEQ_INDEX];
                }

                static const uint16_t METASEQ_INDEX = 149u;
                static const uint16_t METAFRAMECNTR = 150u;

                uint16_t getRequiredImageWidth() const override
                {
                    // This PDI references hardcoded locations up to an offset of METAFRAMECNTR.
                    return METAFRAMECNTR + 1;
                }

            };
        }
    }
}
