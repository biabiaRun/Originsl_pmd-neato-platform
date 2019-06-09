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

#include <common/exceptions/ResourceError.hpp>

namespace royale
{
    namespace common
    {
        /// @brief Could not open exception
        ///
        /// Thrown when a file or a device could not be opened. This could
        /// be due to a permissions issue or other reason.
        /// @sa FileNotFound, DeviceIsBusy
        class CouldNotOpen : public ResourceError
        {
        public:
            CouldNotOpen (std::string dt = "could not open", std::string du = "") : ResourceError (dt, du, royale::CameraStatus::COULD_NOT_OPEN)
            {
            }

            std::string getExceptionType () const override
            {
                return "CouldNotOpen exception";
            }

        };
    }
}
