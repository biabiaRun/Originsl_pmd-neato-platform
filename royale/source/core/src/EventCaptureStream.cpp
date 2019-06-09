/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventCaptureStream.hpp>

using namespace royale::event;


EventCaptureStream::EventCaptureStream (royale::EventSeverity severity, const royale::String &message)
    : m_severity (severity),
      m_message (message)
{
}

royale::EventSeverity EventCaptureStream::severity() const
{
    return m_severity;
}

const royale::String EventCaptureStream::describe() const
{
    return m_message;
}

royale::EventType EventCaptureStream::type() const
{
    return royale::EventType::ROYALE_CAPTURE_STREAM;
}
