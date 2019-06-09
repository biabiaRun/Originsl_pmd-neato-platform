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
        /// @brief Resource error exception
        ///
        /// Parent exception class for errors that deal with resources
        /// like files, devices or data structures.
        class ResourceError : public Exception
        {
        public:
            ResourceError (std::string dt = "ressource error", std::string du = "", royale::CameraStatus s = royale::CameraStatus::RESOURCE_ERROR) : Exception (dt, du, s)
            {
            }

            std::string getExceptionType () const override
            {
                return "ResourceError exception";
            }

        };
    }
}

