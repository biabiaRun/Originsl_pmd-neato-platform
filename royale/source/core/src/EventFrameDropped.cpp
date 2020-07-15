/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventFrameDropped.hpp>
#include <sstream>

using namespace royale::event;

EventFrameDropped::EventFrameDropped (unsigned nrOfFramesDropped) :
    m_nrOfFramesDropped (nrOfFramesDropped)
{
}

royale::EventSeverity EventFrameDropped::severity() const
{
    return royale::EventSeverity::ROYALE_WARNING;
}

const royale::String EventFrameDropped::describe() const
{
    std::ostringstream os;
    os << "Dropped " << m_nrOfFramesDropped << " frame(s).";
    return os.str();
}

royale::EventType EventFrameDropped::type() const
{
    return royale::EventType::ROYALE_FRAME_DROP;
}
