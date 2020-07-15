/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <royale/IEvent.hpp>

namespace royale
{
    namespace event
    {

        /**
        * Events that occur during the recording.
        */
        class EventRecording : public royale::IEvent
        {
        public:
            ROYALE_API EventRecording (const royale::EventSeverity &severity, const royale::String &eventText);

            // implement IEvent
            royale::EventSeverity severity() const override;
            const royale::String describe() const override;
            royale::EventType type() const override;

            royale::EventSeverity m_severity;
            royale::String m_eventText;
        };
    }
}
