/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
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
        /**
         * This exception is thrown when an exception happens in the device, and the device is able
         * to detect the error and provide further diagnostics.
         *
         * Some situations will result in a generic RuntimeError instead.
         */
        class DeviceDetectedError : public royale::common::RuntimeError
        {
        public:
            DeviceDetectedError (std::string dt, uint32_t errorCode, uint32_t secondary = 0) :
                RuntimeError {dt},
                m_errorCode {errorCode},
                m_secondary {secondary}
            {
            }

            ~DeviceDetectedError() = default;

            /**
             * Returns a firmware-dependent number, interpreting these requires reference to the
             * specific device's list of error codes.
             */
            uint32_t getErrorCode() const
            {
                return m_errorCode;
            }

            /**
             * Some errors will have further diagnostic info.  This function returns data that
             * may be valid, but interpreting it requires reference to the specific device's
             * documentation.
             */
            uint32_t getSecondaryCode() const
            {
                return m_secondary;
            }

        private:
            const uint32_t m_errorCode;
            const uint32_t m_secondary;
        };
    }
}
