/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventRecording.hpp>

using namespace royale::event;

EventRecording::EventRecording (const royale::EventSeverity &severity, const royale::String &eventText)
    : m_severity (severity),
      m_eventText (eventText)
{
}

royale::EventSeverity EventRecording::severity() const
{
    return m_severity;
}

const royale::String EventRecording::describe() const
{
    return m_eventText;
}

royale::EventType EventRecording::type() const
{
    return royale::EventType::ROYALE_RECORDING;
}
