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

#include <common/exceptions/RuntimeError.hpp>

namespace royale
{
    namespace common
    {
        /// @brief Invalid value exception
        ///
        /// Thrown when an invalid value is attempted to be used. For example a modulation frequency that is not supported by a device.
        class InvalidValue : public RuntimeError
        {
        public:
            InvalidValue (std::string dt = "invalid value", std::string du = "") : RuntimeError (dt, du, royale::CameraStatus::INVALID_VALUE)
            {
            }

            std::string getExceptionType () const override
            {
                return "InvalidValue exception";
            }

        };
    }
}

