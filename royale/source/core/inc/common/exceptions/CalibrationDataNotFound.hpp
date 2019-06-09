/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 * Unauthorized copying of this file, via any medium is strictly prohibited
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
        /// @brief When trying to read the calibration data, a named item was missing.
        ///
        /// Thrown when some piece of information is not available, by the name
        /// that it was expected to have.  Probably an incompatibility between
        /// royale and the calibration tool.
        class CalibrationDataNotFound : public DataNotFound
        {
        public:
            CalibrationDataNotFound (std::string dt = "", std::string du = "") : DataNotFound (dt, du)
            {
            }

            std::string getExceptionType () const override
            {
                return "CalibrationDataNotFound exception";
            }

        };
    }
}
