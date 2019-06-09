/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/EventForwarder.hpp>


using namespace royale;


EventForwarder::EventForwarder()
    : m_listener (nullptr),
      m_mutex()
{
}

EventForwarder::EventForwarder (const EventForwarder &src)
    : m_listener (nullptr),
      m_mutex()
{
    this->setEventListener (src.getEventListener());
}

EventForwarder &EventForwarder::operator= (const EventForwarder &src)
{
    this->setEventListener (src.getEventListener());
    return *this;
}

void EventForwarder::setEventListener (royale::IEventListener *listener)
{
    std::unique_lock<std::mutex> lock (m_mutex);
    m_listener = listener;
}

royale::IEventListener *EventForwarder::getEventListener() const
{
    std::unique_lock<std::mutex> lock (m_mutex);
    return m_listener;
}

void EventForwarder::sendEvent (std::unique_ptr<royale::IEvent> &&event) const
{
    std::unique_lock<std::mutex> lock (m_mutex);
    if (m_listener)
    {
        m_listener->onEvent (std::move (event));
    }
}

