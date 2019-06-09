/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __ARRAYALLOCATOR_HPP__
#define __ARRAYALLOCATOR_HPP__

#include <cstddef>
#include "common/CommonConfig.hpp"

namespace spectre
{
    namespace common
    {
        template<typename T>
        SPECTRE_COMMON_API T *allocArray (size_t size);

        template<typename T>
        SPECTRE_COMMON_API void freeArray (T *arr);

        namespace details
        {
            template<typename T>
            SPECTRE_COMMON_API T *callCopyCtor (const T &src);

            template<typename T>
            SPECTRE_COMMON_API void callDelete (T *ptr);
        }

    }
}

#endif /*__ARRAYALLOCATOR_HPP__*/
