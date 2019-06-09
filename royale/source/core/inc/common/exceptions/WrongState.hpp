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
        /// @brief Wrong state exception
        ///
        /// Thrown when a function is called while being in the wrong state.
        class WrongState : public royale::common::LogicError
        {
        public:
            WrongState (std::string dt = "wrong state", std::string du = "") : LogicError (dt, du, royale::CameraStatus::NOT_IMPLEMENTED)
            {
            }

            std::string getExceptionType () const override
            {
                return "WrongState exception";
            }

        };
    }
}

