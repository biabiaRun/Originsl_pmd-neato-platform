/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
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
        * This indicates an error with the internal
        * eye safety monitoring
        */
        class EventEyeSafety : public royale::IEvent
        {
        public:
            ROYALE_API EventEyeSafety (const uint32_t eyeError);

            // implement IEvent
            royale::EventSeverity severity() const override;
            const royale::String describe() const override;
            royale::EventType type() const override;

        private:
            royale::EventSeverity m_severity;
            royale::String        m_message;
        };
    }
}
