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
        /// @brief Time out exception
        ///
        /// Thrown when something just takes too long.
        class Timeout : public RuntimeError
        {
        public:
            Timeout (std::string dt = "timeout", std::string du = "") : RuntimeError (dt, du, royale::CameraStatus::TIMEOUT)
            {
            }

            std::string getExceptionType () const override
            {
                return "Timeout exception";
            }

        };
    }
}
