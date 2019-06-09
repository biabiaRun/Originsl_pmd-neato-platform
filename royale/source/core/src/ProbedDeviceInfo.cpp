/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <device/ProbedDeviceInfo.hpp>

namespace royale
{
    namespace device
    {
        const royale::String &ProbedDeviceInfo::getDeviceName() const
        {
            return m_deviceName;
        }

        void ProbedDeviceInfo::setDeviceName (const royale::String &newDeviceName)
        {
            m_deviceName = newDeviceName;
        }
    }
}
