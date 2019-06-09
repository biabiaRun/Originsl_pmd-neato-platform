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

#include <common/IPseudoDataInterpreter.hpp>
#include <royale/Definitions.hpp>

namespace royale
{
    namespace common
    {
        /**
         * The IPseudoDataInterpreter interface has several methods for calculating frame numbers,
         * which can be common to all imagers which use 12-bit numbers.
         */
        class PseudoDataTwelveBitCalculator : public IPseudoDataInterpreter
        {
        public:
            uint16_t getFollowingFrameNumber (uint16_t base, uint16_t n) const override
            {
                return static_cast<uint16_t> ( (base + n) & 0x0FFF);
            }

            bool isGreaterFrame (uint16_t base, uint16_t n) const override
            {
                if (base == n)
                {
                    return false;
                }

                uint32_t nUnwrapped = n;
                if (base > n)
                {
                    nUnwrapped += 0x1000;
                }
                return (nUnwrapped - base) < 0x800;
            }

            uint16_t frameNumberFwdDistance (uint16_t lhs, uint16_t rhs) const override
            {
                return (lhs + 0x1000u - rhs) % 0x1000u;
            }
        };
    }
}
