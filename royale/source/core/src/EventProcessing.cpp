/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventProcessing.hpp>

using namespace royale::event;

EventProcessing::EventProcessing (const royale::EventSeverity &severity, const royale::String &eventText)
    : m_severity (severity),
      m_eventText (eventText)
{
}

royale::EventSeverity EventProcessing::severity() const
{
    return m_severity;
}

const royale::String EventProcessing::describe() const
{
    return m_eventText;
}

royale::EventType EventProcessing::type() const
{
    return royale::EventType::ROYALE_PROCESSING;
}
