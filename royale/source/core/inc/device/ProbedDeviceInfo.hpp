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

#include <royale/String.hpp>

namespace royale
{
    namespace device
    {
        /**
         * This class contains information about one probed device. Add attributes to this class as
         * needed to collect all of the required information about a probed device.
         */
        class ProbedDeviceInfo
        {
        public:
            /**
             * Get the name of the device which this info is about.
             * @return the name of the device.
             */
            const royale::String &getDeviceName() const;
            /**
             * Set the name of the device which this info is about.
             * @param newDeviceName This is the device name to be set.
             */
            void setDeviceName (const royale::String &newDeviceName);
        private:
            royale::String m_deviceName;
        };
    }
}
