/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <common/exceptions/OutOfBounds.hpp>

namespace royale
{
    namespace common
    {
        /*!
         * Narrow cast proposed by Stroustrup's C++ bible (pg. 299, 11.5)
         */
        template<typename Target, typename Source>
        Target narrow_cast (Source v)
        {
            auto r = static_cast<Target> (v);
            if (static_cast<Source> (r) != v)
            {
                throw OutOfBounds ("narrow_cast<>() failed");
            }
            return r;
        }
    }
}
