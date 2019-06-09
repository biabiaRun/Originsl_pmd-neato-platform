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

namespace royale
{
    namespace common
    {
        /// @brief Runtime error exception
        ///
        /// Parent exception class for errors that can happen during
        /// the normal operation of a program.
        class RuntimeError : public Exception
        {
        public:
            RuntimeError (std::string dt = "runtime error", std::string du = "", royale::CameraStatus s = royale::CameraStatus::RUNTIME_ERROR) : Exception (dt, du, s)
            {
            }

            std::string getExceptionType () const override
            {
                return "RuntimeError exception";
            }

        };
    }
}

