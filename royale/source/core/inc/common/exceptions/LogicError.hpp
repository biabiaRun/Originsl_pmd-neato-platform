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

#include <common/exceptions/Exception.hpp>
#include <royale/Status.hpp>

namespace royale
{
    namespace common
    {
        /// @brief Logic error exception
        ///
        /// Parent exception class for errors that happen when a program
        /// does something it is not supposed to do.
        class LogicError : public Exception
        {
        public:
            LogicError (std::string dt = "logic error", std::string du = "", royale::CameraStatus s = royale::CameraStatus::LOGIC_ERROR) : Exception (dt, du, s)
            {
            }

            std::string getExceptionType () const override
            {
                return "LogicError exception";
            }

        };
    }
}

