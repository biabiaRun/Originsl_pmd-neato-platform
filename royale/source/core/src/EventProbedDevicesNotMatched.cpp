/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <common/events/EventProbedDevicesNotMatched.hpp>

namespace royale
{
    namespace event
    {

        ROYALE_API EventProbedDevicesNotMatched::EventProbedDevicesNotMatched()
        {
        }

        royale::EventSeverity EventProbedDevicesNotMatched::severity() const
        {
            return royale::EventSeverity::ROYALE_INFO;
        }

        const royale::String EventProbedDevicesNotMatched::describe() const
        {
            const royale::device::ProbedDeviceInfoList &probedDeviceInfoList
                = m_probeResultInfo.getDeviceInfoList();

            String description;

            if (probedDeviceInfoList.empty())
            {
                description += "Camera devices were found but not matched.\n";
            }
            else
            {
                description += "The following camera devices were found but not matched:\n";

                for (const royale::device::ProbedDeviceInfo &probedDeviceInfo : probedDeviceInfoList)
                {
                    const royale::String &probedDeviceName = probedDeviceInfo.getDeviceName();

                    description += "* ";
                    description += probedDeviceName;
                    description += "\n";
                }
            }

            return description;
        }

        void EventProbedDevicesNotMatched::setProbeResultInfo (
            const royale::device::ProbeResultInfo &newProbeResultInfo)
        {
            m_probeResultInfo = newProbeResultInfo;
        }

        royale::EventType EventProbedDevicesNotMatched::type() const
        {
            return royale::EventType::ROYALE_UNKNOWN;
        }
    }
}
