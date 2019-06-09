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
        /// @brief Disconnected exception
        ///
        /// Thrown when a connection to a device is unexpectedly lost.
        /// @sa CouldNotOpen, DeviceIsBusy
        class Disconnected : public RuntimeError
        {
        public:
            Disconnected (std::string dt = "disconnected", std::string du = "") : RuntimeError (dt, du, royale::CameraStatus::DISCONNECTED)
            {
            }

            std::string getExceptionType () const override
            {
                return "Disconnected exception";
            }

        };
    }
}
