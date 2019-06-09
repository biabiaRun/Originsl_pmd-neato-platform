/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <private/EventListenerCAPI.hpp>
#include <private/HelperFunctionsCAPI.hpp>
#include <royale/IEvent.hpp>


EventListenerCAPI::EventListenerCAPI (ROYALE_EVENT_CALLBACK cb) :
    m_externalEventCallback (cb)
{

}

void EventListenerCAPI::onEvent (std::unique_ptr<royale::IEvent> &&royaleEvent)
{
    if (m_externalEventCallback != nullptr)
    {
        royale_event theEvent;
        EventCAPIConverter::fromRoyaleEvent (std::move (royaleEvent), &theEvent);
        m_externalEventCallback (&theEvent);
    }
}

void EventCAPIConverter::fromRoyaleEvent (std::unique_ptr<royale::IEvent> &&royaleEvent, royale_event *nativeEvent)
{
    nativeEvent->severity = (royale_event_severity) royaleEvent->severity();
    nativeEvent->type = (royale_event_type) royaleEvent->type();
    HelperFunctionsCAPI::copyRoyaleStringToCString (&nativeEvent->description, royaleEvent->describe());
}
