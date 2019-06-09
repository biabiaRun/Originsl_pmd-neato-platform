/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventRawFrameStats.hpp>
#include <sstream>

using namespace royale::event;

EventRawFrameStats::EventRawFrameStats (uint16_t totalFrames, uint16_t frameDropsBridge, uint16_t frameDropsCollector)
    : m_totalFrames (totalFrames),
      m_frameDropsBridge (frameDropsBridge),
      m_frameDropsCollector (frameDropsCollector)
{
}

royale::EventSeverity EventRawFrameStats::severity() const
{
    if (m_frameDropsBridge || m_frameDropsCollector)
    {
        return royale::EventSeverity::ROYALE_WARNING;
    }
    return royale::EventSeverity::ROYALE_INFO;
}

const royale::String EventRawFrameStats::describe() const
{
    std::ostringstream os;
    os << "Raw frame drop stats: "
       << "Bridge " << m_frameDropsBridge << " frames dropped, "
       << "FC " << m_frameDropsCollector << " frames dropped, "
       << m_totalFrames - m_frameDropsBridge - m_frameDropsCollector << " frames delivered."
       ;
    return os.str();
}

royale::EventType EventRawFrameStats::type() const
{
    return royale::EventType::ROYALE_RAW_FRAME_STATS;
}
