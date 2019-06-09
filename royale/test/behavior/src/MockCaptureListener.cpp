#include <MockCaptureListener.hpp>
#include <iostream>

using namespace royale::bdd;
using namespace royale;
using namespace std;


MockExtendedCaptureListener::MockExtendedCaptureListener ()
{
    resetCounters();
}

MockExtendedCaptureListener::~MockExtendedCaptureListener()
{
}

void MockExtendedCaptureListener::onNewData (const royale::IExtendedData *data)
{
    if (m_counterUCDs == 0)
    {
        m_timeMark.reset();

    }

    if (data->hasRawData())
    {
        m_lastTimeStamp = data->getRawData()->timeStamp;
        m_counterFrames = static_cast<int> (data->getRawData()->rawData.size());
        m_frameRateStats.add_timestamp (data->getRawData()->timeStamp);
    }
    m_counterUCDs++;
}

/** Number of captured use cases (equal to number of calls to captureCallback) */
int MockExtendedCaptureListener::getCounterCallbacks()
{
    return m_counterUCDs;
}

/** Number of individual RawFrames in the captured UCDs */
int MockExtendedCaptureListener::getCounterFrames()
{
    return m_counterFrames;
}

royale::common::RoyaleProfiler MockExtendedCaptureListener::getTimeofFirstCallback()
{
    return m_timeMark;
}

std::chrono::microseconds MockExtendedCaptureListener::getLastTimeStamp()
{
    return m_lastTimeStamp;
}

FrameRateStats &MockExtendedCaptureListener::getFrameRateStats()
{
    return m_frameRateStats;
}

void MockExtendedCaptureListener::resetCounters()
{
    m_counterFrames = 0;
    m_counterUCDs = 0;
}

// ------------------------- DepthCallback ---------------------------------

MockDepthCaptureListener::MockDepthCaptureListener ()
{
    resetCounters();
}

MockDepthCaptureListener::~MockDepthCaptureListener()
{
}

void MockDepthCaptureListener::onNewData (const royale::DepthData *data)
{
    if (!m_callbackCounter)
    {
        m_timeMark.reset();
    }

    m_lastTimeStamp = data->timeStamp;
    m_frameRateStats.add_timestamp (data->timeStamp);
    m_Height = data->height;
    m_Width = data->width;
    m_Points = data->points.size();
    m_callbackCounter++;
}

/** Number of captured use cases (equal to number of calls to captureCallback) */
int MockDepthCaptureListener::getCallBackCounter() const
{
    return m_callbackCounter;
}

int MockDepthCaptureListener::getWidth() const
{
    return m_Width;
}

int MockDepthCaptureListener::getHeight() const
{
    return m_Height;
}

std::chrono::microseconds MockDepthCaptureListener::getLastTimeStamp()
{
    return m_lastTimeStamp;
}

FrameRateStats &MockDepthCaptureListener::getFrameRateStats()
{
    return m_frameRateStats;
}

royale::common::RoyaleProfiler MockDepthCaptureListener::getTimeofFirstCallback()
{
    return m_timeMark;
}

void MockDepthCaptureListener::resetCounters()
{
    m_callbackCounter = 0;
}

size_t MockDepthCaptureListener::getPointCount() const
{
    return m_Points;
}
