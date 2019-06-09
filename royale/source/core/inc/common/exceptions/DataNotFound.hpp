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
        /**
         * Thrown when some piece of information is not available in a file, device or data
         * structure.  This is not an I/O error, it indicates that a container could be read, but
         * that container did not contain the requested data.
         */
        class DataNotFound : public ResourceError
        {
        public:
            DataNotFound (std::string dt = "data not found", std::string du = "", royale::CameraStatus s = royale::CameraStatus::DATA_NOT_FOUND) : ResourceError (dt, du, s)
            {
            }

            std::string getExceptionType () const override
            {
                return "DataNotFound exception";
            }

        };
    }
}

