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

#include <imager/M2450_A12/PseudoDataInterpreter.hpp>

namespace royale
{
    namespace imager
    {
        namespace M2450_A11
        {
            /**
            * Converts the pseudodata of the M2450 A11 sensor.
            */
            class PseudoDataInterpreter : public M2450_A12::PseudoDataInterpreter
            {
            public:
                ~PseudoDataInterpreter() = default;

                PseudoDataInterpreter *clone() override
                {
                    return new PseudoDataInterpreter (*this);
                }

                uint16_t getReconfigIndex (const royale::common::ICapturedRawFrame &frame) const override
                {
                    //legacy support - use the frame index as reconfig index
                    return frame.getPseudoData() [0];
                }

                /*
                 * Reminder: When adding/changing anything in this class that accesses pseudodata,
                 * don't forget to check if the base class getRequiredImageWidth() is still adequate.
                 */
                using M2450_A12::PseudoDataInterpreter::getRequiredImageWidth;

                void getEyeSafetyError (const common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
                {
                    eyeError = 0u;
                }
            };
        }
    }
}
