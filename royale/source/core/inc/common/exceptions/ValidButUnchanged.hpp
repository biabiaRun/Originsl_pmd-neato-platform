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

#include <common/exceptions/RuntimeError.hpp>

namespace royale
{
    namespace common
    {
        /// @brief ValidButUnchanged exception
        ///
        /// This exception is thrown when an unexpected error prevents a change,
        /// but the device has been fully recovered to the previous state, as if
        /// the throwing function had not been called. The change was expected to
        /// succeed, calling the function again with exactly the same parameters
        /// is expected to succeed.
        class ValidButUnchanged : public royale::common::RuntimeError
        {
        public:
            ValidButUnchanged (std::string dt = "failed to perform a valid operation and recovered previous state", std::string du = "") :
                RuntimeError (dt, du, royale::CameraStatus::RUNTIME_ERROR)
            {
            }

            std::string getExceptionType () const override
            {
                return "ValidButUnchanged exception";
            }

        };
    }
}

