/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <buffer/BridgeCopyAndNormalize.hpp>
#include <buffer/BufferUtils.hpp>

#include <common/events/EventCaptureStream.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/LogicError.hpp>

#include <algorithm>

using namespace royale::common;
using namespace royale::buffer;
using std::size_t;

BridgeCopyAndNormalize::BridgeCopyAndNormalize (BufferDataFormat format) :
    m_transferFormat {format},
    m_captureStarted {false}
{
}

BridgeCopyAndNormalize::~BridgeCopyAndNormalize() = default;

void BridgeCopyAndNormalize::setTransferFormat (royale::buffer::BufferDataFormat format)
{
    m_transferFormat = format;
}

BufferDataFormat BridgeCopyAndNormalize::getTransferFormat()
{
    return m_transferFormat;
}

std::size_t BridgeCopyAndNormalize::executeUseCase (int imageWidth, int imageHeight, std::size_t bufferCount)
{
    waitCaptureBufferDealloc();

    auto pixelCount = static_cast<std::size_t> (imageWidth * imageHeight);
    // The data is unpacked into these buffers, so the buffer needs to be large enough for
    // normalized data, regardless of which BufferDataFormat is being received.
    auto size = pixelCount * sizeof (uint16_t);
    createAndQueueInternalBuffers (bufferCount, size, 0, pixelCount);

    // createAndQueueInternalBuffers didn't throw an exception, so it has allocated all the buffers
    return bufferCount;
}

bool BridgeCopyAndNormalize::shouldBlockForDequeue()
{
    return m_captureStarted;
}

void BridgeCopyAndNormalize::startCapture()
{
    if (!isConnected ())
    {
        throw LogicError ("Not connected");
    }
    m_captureStarted = true;
}

void BridgeCopyAndNormalize::stopCapture()
{
    m_captureStarted = false;
}

void BridgeCopyAndNormalize::bridgeAcquisitionCallback (
    const size_t sampleSize,
    const void *data,
    uint64_t timestamp)
{
    if (!m_captureStarted)
    {
        // stopCapture() has been called, just drop the frame
        return;
    }

    auto frame = dequeueInternalBuffer();
    if (!frame)
    {
        return;
    }

    // If the format is unknown, try to auto-detect it. Once the format has been auto-detected, the
    // bridge will keep using the same format, so that a mixed-mode with variable-size superframes
    // doesn't end up with one size being correctly detected, and the other size incorrectly detected.
    //
    // For Arctic firmware:
    // version v0.8 and later has CX3 using RAW12, but FX3 using RAW16
    // version v0.13.1 and later: IDeviceStatus::getUsbTransferFormat() is supported
    if (m_transferFormat == BufferDataFormat::UNKNOWN)
    {
        // Check for Raw16 first, as most of the testing will be on Raw12 devices
        const auto pixelCount = frame->getPixelCount();
        if (sampleSize == BufferUtils::expectedRawSize (pixelCount, BufferDataFormat::RAW16))
        {
            m_transferFormat = BufferDataFormat::RAW16;
        }
        else if (sampleSize == BufferUtils::expectedRawSize (pixelCount, BufferDataFormat::RAW12))
        {
            m_transferFormat = BufferDataFormat::RAW12;
        }
        else
        {
            LOG (WARN) << "Can't auto-detect data format from sample size " << sampleSize;
            m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING,
                    "Can't auto-detect data format from sample size");
        }
    }

    if (m_transferFormat != BufferDataFormat::UNKNOWN)
    {
        BufferUtils::copyAndNormalize (*frame, data, sampleSize, m_transferFormat);
    }

    frame->setTimeMicroseconds (timestamp);

    try
    {
        bufferCallback (frame);
    }
    catch (const Exception &e)
    {
        // bufferCallback() can fail intermittently due to truncated buffers delivered by the system.
        // As this is a temporary issue and capturing continues afterwards, it's EVENT_SEVERITY_WARNING only.
        m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING,
                e.getTechnicalDescription());
    }
}

void BridgeCopyAndNormalize::setEventListener (royale::IEventListener *listener)
{
    m_eventForwarder.setEventListener (listener);
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeCopyAndNormalize::getBridgeInfo()
{
    return royale::Vector<royale::Pair<royale::String, royale::String>>();
}
