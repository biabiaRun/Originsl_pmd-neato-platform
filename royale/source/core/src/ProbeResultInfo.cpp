/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <device/ProbeResultInfo.hpp>

namespace royale
{
    namespace device
    {
        void ProbeResultInfo::addDeviceInfo (const ProbedDeviceInfo &probedDeviceInfo)
        {
            m_probedDeviceInfos.push_back (probedDeviceInfo);
        }

        bool ProbeResultInfo::devicesWereFound() const
        {
            return !m_probedDeviceInfos.empty();
        }

        const ProbedDeviceInfoList &ProbeResultInfo::getDeviceInfoList() const
        {
            return m_probedDeviceInfos;
        }
    }
}
