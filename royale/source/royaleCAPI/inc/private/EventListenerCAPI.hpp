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

#include <EventCAPI.h>
#include <royale/IEventListener.hpp>

class EventListenerCAPI : public royale::IEventListener
{
public:
    explicit EventListenerCAPI (ROYALE_EVENT_CALLBACK cb);

    void onEvent (std::unique_ptr<royale::IEvent> &&royaleEvent) override;

private:
    ROYALE_EVENT_CALLBACK m_externalEventCallback;
};

class EventCAPIConverter
{
public:
    static void fromRoyaleEvent (std::unique_ptr<royale::IEvent> &&royaleEvent, royale_event *nativeEvent);
};
