/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <common/exceptions/DataNotFound.hpp>

namespace royale
{
    namespace common
    {
        /**
         * This indicates that I/O was successful, and the data that is there can be recognised, but
         * it's not the data that was expected.  For example, when trying to read data in Zwetschge
         * format, but finding the magic number for Polar instead.
         */
        class WrongDataFormatFound : public DataNotFound
        {
        public:
            WrongDataFormatFound (std::string dt = "", std::string du = "") : DataNotFound (dt, du, royale::CameraStatus::WRONG_DATA_FORMAT_FOUND)
            {
            }

            std::string getExceptionType () const override
            {
                return "WrongDataFormatFound exception";
            }
        };
    }
}
