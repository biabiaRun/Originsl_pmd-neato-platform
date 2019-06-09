/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventEyeSafety.hpp>

#include <iomanip>

using namespace royale::event;


EventEyeSafety::EventEyeSafety (const uint32_t eyeError)
    : m_severity (royale::EventSeverity::ROYALE_FATAL)
{
    std::stringstream ss;
    ss << "eyeError : 0x" << std::setfill ('0') << std::setw (8) << std::uppercase << std::hex << eyeError;

    m_message = ss.str();
}

royale::EventSeverity EventEyeSafety::severity() const
{
    return m_severity;
}

const royale::String EventEyeSafety::describe() const
{
    return m_message;
}

royale::EventType EventEyeSafety::type() const
{
    return royale::EventType::ROYALE_EYE_SAFETY;
}
