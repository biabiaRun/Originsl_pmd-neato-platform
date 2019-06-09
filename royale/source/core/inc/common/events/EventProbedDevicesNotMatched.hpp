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

#include <device/ProbeResultInfo.hpp>
#include <royale/IEvent.hpp>

namespace royale
{
    namespace event
    {
        /**
         * This class represents the event which occurs when probed devices were found but not
         * matched to those recognized by Royale.
         */
        class EventProbedDevicesNotMatched : public royale::IEvent
        {
        public:
            /**
             * Create this event.
             */
            ROYALE_API EventProbedDevicesNotMatched();
            royale::EventSeverity severity() const override;

            /**
             * @copydoc royale::IEvent::describe
             *
             * If the probe result info is set, the returned event description contains the names of
             * the devices which were found but not matched.
             */
            const royale::String describe() const override;

            /**
             * Set the probe result info of this event. If the probe result info is set, the
             * event description returned from describe() has more detail.
             * @param newProbeResultInfo the probe result info which is to be set.
             */
            ROYALE_API void setProbeResultInfo (
                const royale::device::ProbeResultInfo &newProbeResultInfo);

            royale::EventType type() const override;
        private:
            royale::device::ProbeResultInfo m_probeResultInfo;
        };
    }
}
