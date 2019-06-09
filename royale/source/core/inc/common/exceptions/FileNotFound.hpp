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
        /// @brief File not found exception
        ///
        /// Thrown when a file does not exist.
        /// @sa CouldNotOpen, DataNotFound
        class FileNotFound : public royale::common::ResourceError
        {
        public:
            FileNotFound (std::string dt = "file not found", std::string du = "") : ResourceError (dt, du, royale::CameraStatus::FILE_NOT_FOUND)
            {
            }

            std::string getExceptionType () const override
            {
                return "FileNotFound exception";
            }

        };
    }
}
