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

#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * Return the smallest multiple of unit which is equal or larger than numerator.
         */
        inline std::size_t roundUpToUnit (std::size_t numerator, std::size_t unit)
        {
            const auto remainder = numerator % unit;
            if (remainder == 0)
            {
                return numerator;
            }

            return numerator + unit - remainder;
        }

        /**
         * Calculate numerator/denominator, but if there would be a fraction left over in
         * non-integer division then round up instead of down.
         */
        inline std::size_t divideRoundUp (std::size_t numerator, std::size_t denominator)
        {
            if (numerator % denominator == 0)
            {
                return numerator / denominator;
            }
            else
            {
                return (numerator / denominator) + 1;
            }
        }

        /**
        * Wrap-round arithmetic, return the sum of a + b, but wrap it back to zero when it
        * reaches the second argument.
        */
        template <typename T>
        inline T circularAdd (T a, T b, T modulus)
        {
            return (a + b) % modulus;
        }

        /**
         * Wrap-round arithmetic, increment the counter, but wrap it back to zero when it
         * reaches the second argument.  This assumes that the counter will normally be
         * in the expected ranger, and so needing to do a modulus operation is unlikely.
         */
        template <typename T>
        inline void circularIncrement (T &counter, T modulus, T increment = 1)
        {
            if (counter + increment < modulus)
            {
                counter += increment;
            }
            else if (counter + increment == modulus)
            {
                counter = 0;
            }
            else
            {
                counter = circularAdd (counter, increment, modulus);
            }
        }

        /**
         * Move a pointer around a circular buffer, assuming that the pointer is currently a valid
         * pointer in to the buffer (or to the first byte after the buffer).
         *
         * If ptr == end && increment == zero, then ptr will be set to begin.
         */
        template <typename T>
        inline void circularBufferIncrement (T *&ptr, T *begin, T *end, std::size_t increment)
        {
            const auto distance = std::distance (ptr, end);
            if (increment < static_cast<std::size_t> (distance))
            {
                ptr += increment;
            }
            else
            {
                ptr = begin + increment - distance;
            }
        }
    }
}
