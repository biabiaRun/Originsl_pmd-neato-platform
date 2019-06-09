/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/OutOfBounds.hpp>
#include <common/NarrowCast.hpp>
#include <iterator>

#pragma once

namespace royale
{
    namespace common
    {
        /** Helper for calculating the return type and return value of iteratorBoundsCheck */
#ifdef WIN32
        template<typename T>
        inline stdext::checked_array_iterator<T *> royale_make_possibly_checked_iterator (T *first, std::size_t minSize)
        {
            return stdext::make_checked_array_iterator (first, minSize);
        }
#else
        template<typename T>
        inline T royale_make_possibly_checked_iterator (T first, std::size_t minSize)
        {
            return first;
        }
#endif

        /**
         * Check that a pair of [first, last) iterators provide access to at least minSize elements,
         * throwing an exception if they don't.  This minimum size is the number of elements that
         * the caller intends to access, so it's not an error for std::distance (first, last) to be
         * more than the expected size.
         *
         * This is intended for code that accessing subparts of buffers by passing pairs of
         * (first, last) pointers to a function.
         *
         * This returns an iterator which is valid for the range [first, first + minSize).  On
         * Windows, it returns something compatible with Microsoft's checked iterator framework,
         * which will detect if an out-of-bounds access is made, and therefore can be used as the
         * destination of std::copy without triggering Microsoft's C4996 family of warnings (the
         * ones disabled by _SCL_SECURE_NO_WARNINGS).
         *
         * On non-Windows it simply returns the original iterator unchanged.
         *
         * This function only supports pointers, because royale_make_possibly_checked_iterator
         * is currently only implemented for pointers.
         */
        template<typename T>
        inline auto iteratorBoundsCheck (T first, T last, std::size_t minSize)
        -> decltype (royale_make_possibly_checked_iterator (first, 0))
        {
            if (std::distance (first, last) < 0)
            {
                throw LogicError ("Range has reversed start and end");
            }
            else if (narrow_cast<std::size_t> (std::distance (first, last)) < minSize)
            {
                throw OutOfBounds ("Buffer too small");
            }
            return royale_make_possibly_checked_iterator (first, minSize);
        }
    }
}
