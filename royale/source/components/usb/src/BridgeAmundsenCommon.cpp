/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/AmundsenCircularBuffer.hpp>
#include <usb/bridge/BridgeAmundsenCommon.hpp>

#include <common/events/EventCaptureStream.hpp>
#include <common/events/EventDeviceDisconnected.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/Timeout.hpp>

#include <common/EndianConversion.hpp>
#include <common/IntegerMath.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

#include <buffer/BufferUtils.hpp>

#include <algorithm>

using namespace royale::buffer;
using namespace royale::common;
using namespace royale::usb::bridge;
using std::size_t;

namespace
{
    /**
     * UVC payloads have a header that's 12 or more bytes, for all of the Arctic devices the minimum
     * size is used.
     */
    const std::size_t AMUNDSEN_UVC_HEADER_SIZE = 12;

    /**
     * This is used for calculating the number of strides that should be allocated given a known
     * data size. The lower this value, the more strides (and thus overhead) will be allocated.
     *
     * We assume that any device firmware will ensure that each payload contains at least this many
     * bytes of data in addition to the headers; the final payload is an exception, it can be
     * smaller.  Sizes used by the existing firmware are as documented for AMUNDSEN_STRIDE_SIZE.
     */
    const std::size_t MIN_PAYLOAD_DATA_SIZE = 0x2c00;

    static_assert (MIN_PAYLOAD_DATA_SIZE <= 0x2fd0,
                   "minimum payload is larger than the CX3 Arctic devices' output");
    static_assert (MIN_PAYLOAD_DATA_SIZE + AMUNDSEN_UVC_HEADER_SIZE < BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE,
                   "minimum payload data wouldn't fit in the receiving strides");

    /**
     * Thrown to indicate that the current frame's data is wrong, but data capture should continue
     * with the next frame.
     */
    class CurrentFrameIsCorrupt : public RuntimeError
    {
    public:
        explicit CurrentFrameIsCorrupt (std::string dt) :
            RuntimeError (std::move (dt))
        {
        }

        ~CurrentFrameIsCorrupt() = default;
    };

    /**
     * Interpreter for the header of a UVC payload.
     */
    std::size_t getDataOffset (const uint8_t *payload)
    {
        std::size_t headerLength = payload [0];
        if (headerLength != AMUNDSEN_UVC_HEADER_SIZE)
        {
            // \todo: handle this properly
            // This is getting hit on Android
            //
            // The failure is currently happening when receivePayload has received many payloads of
            // size 2fdc, with an occasional odd size (3fdc, 39dc), but the last payload received is
            // 4000 bytes with the first four bytes being 7e, 7f, ca, 80
            //
            // But the number of payloads received seems very low - about 108 in about 2 seconds,
            // running a MODE9_5FPS use case. The expected data would be double that.
            LOG (DEBUG) << "Unexpected data in header:"
                        << " 0x" << std::hex << unsigned (payload[0])
                        << " 0x" << std::hex << unsigned (payload[1])
                        << " 0x" << std::hex << unsigned (payload[2])
                        << " 0x" << std::hex << unsigned (payload[3]);
            throw CurrentFrameIsCorrupt ("Unexpected data in header");
        }
        return headerLength;
    }

    /**
     * Interpreter for the header of a UVC payload, returns the
     * FID, which toggles every time the frame changes.
     */
    bool getFrameIdentifier (const uint8_t *payload)
    {
        return (payload[1] & 0x01) != 0;
    }

    /**
     * Interpreter for the header of a UVC payload, returns the
     * end of frame marker.
     */
    bool isEndOfFrame (const uint8_t *payload)
    {
        return (payload[1] & 0x02) != 0;
    }

#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
    void debugDumpHeader (const uint8_t *payload)
    {
        // \todo: remove this debugging, but it's still useful at the moment
        // Assume the payload always has at least 12 bytes, which is the size that Amundsen always
        // uses; the buffer will always be big enough.
        LOG (DEBUG) << "Data in header:"
                    << " 0x" << std::hex << unsigned (payload[0])
                    << " 0x" << std::hex << unsigned (payload[1])
                    << " 0x" << std::hex << unsigned (payload[2])
                    << " 0x" << std::hex << unsigned (payload[3])
                    << " 0x" << std::hex << unsigned (payload[4])
                    << " 0x" << std::hex << unsigned (payload[5])
                    << " 0x" << std::hex << unsigned (payload[6])
                    << " 0x" << std::hex << unsigned (payload[7])
                    << " 0x" << std::hex << unsigned (payload[8])
                    << " 0x" << std::hex << unsigned (payload[9])
                    << " 0x" << std::hex << unsigned (payload[10])
                    << " 0x" << std::hex << unsigned (payload[11])
                    << " 0x" << std::hex << unsigned (payload[12]);
    }
#endif
}

BridgeAmundsenCommon::BridgeAmundsenCommon() = default;
BridgeAmundsenCommon::~BridgeAmundsenCommon() = default;

void BridgeAmundsenCommon::setUvcBridgeInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &&info)
{
    m_uvcBridgeInfo = info;
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeAmundsenCommon::getBridgeInfo()
{
    decltype (getBridgeInfo()) info;
    for (const auto &x : m_uvcBridgeInfo)
    {
        info.emplace_back (x);
    }
    info.emplace_back ("BRIDGE_TYPE", "Amundsen");
    info.emplace_back ("USB_SPEED", royale::usb::pal::UsbSpeedUtils::getPrintableName (m_usbSpeed));
    return info;
}

void BridgeAmundsenCommon::setEventListener (royale::IEventListener *listener)
{
    m_eventForwarder.setEventListener (listener);
}

void BridgeAmundsenCommon::setTransferFormat (royale::buffer::BufferDataFormat format)
{
    m_transferFormat = format;
}

void BridgeAmundsenCommon::setUsbSpeed (royale::usb::pal::UsbSpeed speed)
{
    m_usbSpeed = speed;
}

float BridgeAmundsenCommon::getPeakTransferSpeed()
{
    if (!isConnected())
    {
        throw Disconnected();
    }

    // Assume the worst case of 16 bits per pixel, instead of the detected format.  Further testing
    // of the flow control is needed before changing to using getTransferFormat().
    return royale::usb::pal::UsbSpeedUtils::calculatePeakTransferSpeed (m_usbSpeed, royale::buffer::BufferDataFormat::RAW16);
}

std::size_t BridgeAmundsenCommon::executeUseCase (int imageWidth, int imageHeight, std::size_t bufferCount)
{
    // There's a reliance on stopCapture() first, so that the acquisition thread isn't holding a
    // buffer.  RoyaleCore always does stop the capture, so this can be relied on.
    if (m_runAcquisition)
    {
        throw NotImplemented ("BridgeAmundsen only supports changing use case when stopped");
    }

    // This is called from the FrameCollector, the FrameCollector frees its own
    // buffers, we just need to wait for it.
    waitCaptureBufferDealloc();

    // The UVC protocol's overhead goes in each received block (of up to 16kB), so we have to remove
    // it while receiving the data.  For the data that we copy to the buffer, the protocol overhead
    // is omitted, so the only data in the buffer is the pixels.
    const size_t pixelOffset = 0;
    const size_t pixelCount = imageWidth * imageHeight;
    const size_t bufferSize = pixelCount * sizeof (uint16_t);

    m_rawDataSize = BufferUtils::maxRawSize (pixelCount, m_transferFormat);

    // Calculate the size for a single buffer which can hold all of the received data, including the
    // headers and the overhead of the non-element parts of each stride.
    if (m_rawDataSize < MIN_PAYLOAD_DATA_SIZE)
    {
        // Accept devices or unit tests that use small buffer sizes for small images (at the time of
        // writing, this is only used for the unit tests).
        m_receiveBufferStrides = 2;
    }
    else
    {
        // The + 1 is for the final payload
        m_receiveBufferStrides = (m_rawDataSize / MIN_PAYLOAD_DATA_SIZE) + 1;
    }

    createAndQueueInternalBuffers (bufferCount, bufferSize, pixelOffset, pixelCount);

    // as createAndQueueInternalBuffers didn't throw, we have all the requested buffers
    return bufferCount;
}

void BridgeAmundsenCommon::startCapture()
{
    std::lock_guard<std::mutex> lock (m_acquisitionStartStopLock);
    if (m_runAcquisition)
    {
        throw LogicError ("Acquisition thread is already running");
    }

    m_runAcquisition = true;
    m_acquisitionThread = std::thread (&BridgeAmundsenCommon::acquisitionFunction, this);
}

void BridgeAmundsenCommon::stopCapture()
{
    std::lock_guard<std::mutex> lock (m_acquisitionStartStopLock);
    if (m_runAcquisition)
    {
        m_runAcquisition = false;
        unblockDequeueThread();
        m_acquisitionThread.join();
    }
}

bool BridgeAmundsenCommon::shouldBlockForDequeue()
{
    return m_runAcquisition;
}

void BridgeAmundsenCommon::acquisitionFunction()
{
    // With UVC, the data for a frame is split in to a series of payloads, each about 16k, with each
    // payload having a header. So we receive these payloads and then copy the data in to the buffer
    // for the complete frame.
    if (!m_receiveBufferStrides)
    {
        LOG (ERROR) << "Trying to receive data without knowing the image size, acquisitionFunction quitting";
        return;
    }
    auto receiveBuffer = AmundsenCircularBuffer (m_receiveBufferStrides);

    // If the FID toggles, then we've received part of a new frame, possibly without seeing the
    // previous end-of-frame.  In UVC the EOF marker is optional, in Amundsen it's always present.
    bool currentFrameIdentifier = false;

    // In this protocol, each payload is the same size, except for the final one that has the
    // end-of-frame marker. It's constant for a device, for simplicity the code in this function
    // allows it to be set only when startOfFrame == nullptr (so the first payload of each frame).
    std::size_t firstPayloadSize = 0;

    // The next data capture goes in to this buffer
    OffsetBasedCapturedBuffer *frame = nullptr;

    // If the acquisition thread catches an unexpected exception (possibly a Disconnect) then the
    // thread should stop.  But leave m_runAcquisition as true so that the thread cleanup and join()
    // is still done.
    bool keepRunning = true;

    while (m_runAcquisition && keepRunning)
    {
        try
        {
            if (!frame)
            {
                frame = dequeueInternalBuffer();
                if (!frame)
                {
                    // We're probably stopping after stopCapture has been called.  If it hasn't,
                    // have a short delay so that this doesn't become a busy loop.
                    if (m_runAcquisition)
                    {
                        LOG (ERROR) << "No free buffers from BridgeInternalBufferAlloc";
                        m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING, "No free buffers from BridgeInternalBufferAlloc");
                        std::chrono::microseconds usec (100000);
                        std::this_thread::sleep_for (usec);
                    }
                    continue;
                    // In the device, data starts being discarded if we're not reading it.  There
                    // will be a dropped frame when we start reading again, but that is handled by the
                    // existing error handling for dropped frames.
                }
                receiveBuffer.clearStartOfFrame();
            }

            if (!receiveBuffer.startOfFrame())
            {
                // assume we're between frames, and have time to set up buffers
                auto status = PrepareStatus::SUCCESS;
                while (status == PrepareStatus::SUCCESS && receiveBuffer.hasUnprepared())
                {
                    status = prepareToReceivePayload (receiveBuffer.firstUnprepared());
                    if (status == PrepareStatus::SUCCESS)
                    {
                        receiveBuffer.setPrepared();
                    }
                }
            }

            if (!receiveBuffer.hasPrepared())
            {
                // Prepare a single transfer, for the I/O that will be used in the immediately following receivePayload() call
                auto status = prepareToReceivePayload (receiveBuffer.firstUnprepared());
                if (status == PrepareStatus::SUCCESS)
                {
                    receiveBuffer.setPrepared();
                }
                else
                {
                    throw RuntimeError ("Can't queue any USB receive buffers");
                }
            }

            // even if the transfer failed (which we won't know until the receivePayload call), the nextPayload moves on
            const auto payload = receiveBuffer.firstPrepared();
            receiveBuffer.setUsed();

            const auto receivedSize = receivePayload (payload, m_runAcquisition);
            if (receivedSize == 0)
            {
                // A timeout is a recoverable error, just run the loop again to retry the read (and
                // this will also check m_runAcquisition to see if the thread should stop).
                //
                // However, with asynchronous I/O multiple payloads have already been queued to
                // receive data, and where the next data will arrive depends on the platform. so the
                // next data received will arrive in one of those payloads.  If the timeout occured
                // in the middle of a frame then this would leave a one-stride gap in the data.
                // Currently this code assumes that the current frame is corrupted, and simply
                // starts the next frame after the gap.
                //
                // If we need to handle long delays in superframes then this needs to change,
                // however it would also need to be handled in embedded bridges, so the better
                // option is probably to ensure that long delays don't occur in superframes.
                // \todo ROYAL-2247 Align superframe gaps to timing
                LOG (WARN) << "receivePayload() returned zero, probably timed out";
                receiveBuffer.clearStartOfFrame();
                continue;
            }
            const auto pixelDataOffset = getDataOffset (payload);
            if (receivedSize < pixelDataOffset)
            {
                LOG (WARN) << "receivePayload() returned a small number of bytes";
                continue;
            }
#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
            else if (receivedSize != 0x2fdc && receivedSize != 0x3ff0)
            {
                LOG (DEBUG) << "receivePayload() returned " << receivedSize << " bytes, frame id " << getFrameIdentifier (payload);
                LOG (DEBUG) << "(frames of sizes 0x2fdc and 0x3ff0 are not logged)";
                debugDumpHeader (payload);
            }
#endif

            const bool handlingFirstPayloadOfFrame = !receiveBuffer.startOfFrame();
            if (handlingFirstPayloadOfFrame)
            {
                receiveBuffer.setCurrentBufferIsFrameStart();
                firstPayloadSize = receivedSize;
            }
            else if (firstPayloadSize != receivedSize && ! isEndOfFrame (payload))
            {
                LOG (WARN) << "Odd-sized payload, and not end of frame";
                receiveBuffer.clearStartOfFrame();
                continue;
            }

            if (currentFrameIdentifier != getFrameIdentifier (payload))
            {
                if (handlingFirstPayloadOfFrame)
                {
                    // This is the first payload of this frame, so the identifier is expected to
                    // toggle
                    currentFrameIdentifier = getFrameIdentifier (payload);
                }
                else
                {
                    LOG (WARN) << "FrameIdentifier changed without receiving an end-of-frame";
                    receiveBuffer.clearStartOfFrame();
                    continue;
                }
            }

            // Check that the converted data will fit in *frame, this check can fail if an
            // end-of-frame packet has been missed in the USB stream.
            //
            // If the format still unknown (waiting for auto-detection), then this checks that it
            // could fit in *frame, assuming auto-detection found it was RAW16.
            const auto rawDataSize =
                (firstPayloadSize - AMUNDSEN_UVC_HEADER_SIZE) * receiveBuffer.countUsedStridesExcludingCurrentBuffer()
                + (receivedSize - AMUNDSEN_UVC_HEADER_SIZE);

            if (rawDataSize > m_rawDataSize)
            {
                // We haven't reached the end of the frame, but another read wouldn't fit in the
                // frame.
                //
                // The error handling here will be to discard the data, by restarting at the start
                // of the frame.  The error state will only finish when we see the next end-of-frame
                // marker.
                LOG (ERROR) << "Read more data than can fit in to the receive buffer, and wrapped on a "
                            << receivedSize << " byte final payload";
                receiveBuffer.clearStartOfFrame();
                continue;
            }

            if (isEndOfFrame (payload))
            {
                if (m_transferFormat == BufferDataFormat::UNKNOWN)
                {
                    // Check for Raw16 first, as most of the testing will be on Raw12 devices
                    const auto pixelCount = frame->getPixelCount();
                    if (rawDataSize == BufferUtils::expectedRawSize (pixelCount, BufferDataFormat::RAW16))
                    {
                        m_transferFormat = BufferDataFormat::RAW16;
                    }
                    else if (rawDataSize == BufferUtils::expectedRawSize (pixelCount, BufferDataFormat::RAW12))
                    {
                        m_transferFormat = BufferDataFormat::RAW12;
                    }
                    else
                    {
                        LOG (WARN) << "Can't auto-detect data format from data size " << rawDataSize;
                        m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING,
                                "Can't auto-detect data format from data size");
                        // Wait for the next frame instead
                        receiveBuffer.clearStartOfFrame();
                        continue;
                    }
                }

                // BufferUtils::copyAndNormalizeStrides copies the data and skips over the protocol
                // headers.  Its view of the buffer is that each stride is the data from one
                // payload, and the headers from the following payload.
                if (receiveBuffer.isWrapped())
                {
                    // Use the circular-buffer version
                    BufferUtils::copyAndNormalizeStrides (*frame,
                                                          receiveBuffer.startOfFrame() + AMUNDSEN_UVC_HEADER_SIZE,
                                                          receiveBuffer.end(),
                                                          receiveBuffer.start() + AMUNDSEN_UVC_HEADER_SIZE,
                                                          receiveBuffer.currentBuffer() + receivedSize,
                                                          firstPayloadSize - AMUNDSEN_UVC_HEADER_SIZE, AMUNDSEN_STRIDE_SIZE,
                                                          m_transferFormat);
                }
                else
                {
                    BufferUtils::copyAndNormalizeStrides (*frame,
                                                          receiveBuffer.startOfFrame() + AMUNDSEN_UVC_HEADER_SIZE,
                                                          receiveBuffer.currentBuffer() + receivedSize,
                                                          firstPayloadSize - AMUNDSEN_UVC_HEADER_SIZE, AMUNDSEN_STRIDE_SIZE,
                                                          m_transferFormat);
                }
                receiveBuffer.clearStartOfFrame();
                currentFrameIdentifier = !currentFrameIdentifier;
                bufferCallback (frame); // may throw
                frame = nullptr;
            }
        }
        catch (const Disconnected &)
        {
            m_eventForwarder.event<royale::event::EventDeviceDisconnected>();

            LOG (ERROR) << "USB Device disconnected.";
            keepRunning = false;
        }
        catch (const CurrentFrameIsCorrupt &)
        {
#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
            LOG (WARN) << "Discarding corrupt frame, which had " << receiveBuffer.countUsedStridesExcludingCurrentBuffer() << " strides before the current payload";
#endif
            receiveBuffer.clearStartOfFrame();
        }
        catch (const Exception &e)
        {
            m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_ERROR, e.getUserDescription());
            LOG (ERROR) << "Caught an exception in the acquisition thread, no more frames will be captured";
            keepRunning = false;
        }
    }

    cancelPendingPayloads();

    if (frame)
    {
        // don't use the IBufferCaptureListener, its thread may have already finished
        queueBuffer (frame);
    }
}
