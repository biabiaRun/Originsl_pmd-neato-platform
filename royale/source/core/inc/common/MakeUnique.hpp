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

#include <memory>

namespace royale
{
    namespace common
    {
        /*
        * Back-ported version of std::make_unique (from C++14) for use with C++11.
        * Ths implementation is taken from MSVC header memory and slightly adapted.
        */


        /**
        * Create an unique_ptr (version for non-array types).
        * Arguments are forwarded to the appropriate constructor for the type.
        */
        template<class _Ty,
                 class... _Types> inline
        typename std::enable_if < !std::is_array<_Ty>::value,
                 std::unique_ptr<_Ty> >::type makeUnique (_Types &&... _Args)
        {
            return std::unique_ptr<_Ty> (new _Ty (std::forward<_Types> (_Args)...));
        }

        /**
        * Create an unique_ptr (version for unbounded arrays).
        * Expects a single argument, the number of elements to allocate.
        * Use like this:
        * auto x = makeUnique<int[]>(14);
        *
        */
        template<class _Ty> inline
        typename std::enable_if < std::is_array<_Ty>::value &&std::extent<_Ty>::value == 0,
                 std::unique_ptr<_Ty> >::type makeUnique (size_t _Size)
        {
            typedef typename std::remove_extent<_Ty>::type _Elem;
            return std::unique_ptr<_Ty> (new _Elem[_Size]());
        }

        /**
        * In C++14, std::make_unique for bounded arrays is explicitly deleted.
        */
        template<class _Ty,
                 class... _Types>
        typename std::enable_if < std::extent<_Ty>::value != 0,
                 void >::type makeUnique (_Types &&...) = delete;

    }
}
