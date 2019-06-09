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

#include <royale/Definitions.hpp>
#include <royale/IEvent.hpp>

namespace royale
{
    namespace event
    {

        /**
        * The photon emitter has experienced a thermal excursion.
        */
        class EventOverTemperature : public royale::IEvent
        {
        public:
            ROYALE_API EventOverTemperature (royale::EventSeverity severity, float value, float limit);

            // implement IEvent
            royale::EventSeverity severity() const override;
            const royale::String describe() const override;
            royale::EventType type() const override;

        private:
            royale::EventSeverity m_severity;
            float m_value;
            float m_limit;
        };
    }
}
