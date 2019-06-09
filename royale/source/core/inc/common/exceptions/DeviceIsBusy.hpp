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
        /// @brief Device is busy exception
        ///
        /// Thrown when a device can not be used because it is otherwide
        /// occupied.
        class DeviceIsBusy : public ResourceError
        {
        public:
            DeviceIsBusy (std::string dt = "device is busy", std::string du = "") : ResourceError (dt, du, royale::CameraStatus::DEVICE_IS_BUSY)
            {
            }

            virtual std::string getExceptionType () const
            {
                return "DeviceIsBusy exception";
            }

        };
    }
}
