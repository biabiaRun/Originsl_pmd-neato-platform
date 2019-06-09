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
        /// @brief Not implemented exception
        ///
        /// Thrown when a function that is not (yet) implemented is called.
        class NotImplemented : public LogicError
        {
        public:
            NotImplemented (std::string dt = "not implemented", std::string du = "") : LogicError (dt, du, royale::CameraStatus::NOT_IMPLEMENTED)
            {
            }

            std::string getExceptionType () const override
            {
                return "NotImplemented exception";
            }

        };
    }
}

