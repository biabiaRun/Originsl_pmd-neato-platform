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

#include <common/exceptions/LogicError.hpp>

namespace royale
{
    namespace common
    {
        /// @brief Out of bounds exception
        ///
        /// Thrown when an index to something is not withing the minimum and maximum position of that data structure. This could, for example, be a file or an array.
        /// @sa InvalidValue, EndOfData, DataNotFound
        class OutOfBounds : public LogicError
        {
        public:
            OutOfBounds (std::string dt = "out of bounds", std::string du = "") : LogicError (dt, du, royale::CameraStatus::OUT_OF_BOUNDS)
            {
            }

            std::string getExceptionType () const override
            {
                return "OutOfBounds exception";
            }

        };
    }
}

