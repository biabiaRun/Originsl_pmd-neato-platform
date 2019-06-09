/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <MockProcessingListeners.hpp>

#include <thread>

using namespace royale::stub::processing;

void MockDepthDataListener::onNewData (const royale::DepthData *data)
{
    // The only reason that this is using ThreadedAssertSupport instead of gmock is that
    // gmock isn't thread-safe on Windows.
}

void SequenceCheckingDepthDataListener::onNewData (const royale::DepthData *data)
{
    if (m_lastTimestamp.count (data->streamId))
    {
        assertTrue (m_lastTimestamp.at (data->streamId) <= data->timeStamp, "non-sequential timestamp");
    }
    m_lastTimestamp[data->streamId] = data->timeStamp;

    // Slow down the callback, so that the Processing has to allocate all of its
    // buffers
    std::this_thread::sleep_for (std::chrono::milliseconds (1));
}
