/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royalev4l/bridge/BridgeV4l.hpp>
#include <buffer/BufferUtils.hpp>
#include <buffer/OffsetBasedCapturedBuffer.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/Timeout.hpp>

#include <common/exceptions/DeviceIsBusy.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <common/events/EventCaptureStream.hpp>

#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <array>
#include <atomic>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


using namespace royale::buffer;
using namespace royale::common;
using namespace royale::v4l::bridge;
using std::size_t;

namespace
{
    /** Value for m_deviceHandle before openConnection */
    const int NOT_CONNECTED = -1;

    /**
     * Holder for one V4L buffer.
     *
     * The construction isn't RAII because it needs to check for the data pointer not being
     * MAP_FAILED before creating the OffsetBasedCapturedBuffer instance, but the destruction is the
     * RAII pattern.
     */
    class CapturedV4lBuffer : public OffsetBasedCapturedBuffer
    {
    public:
        CapturedV4lBuffer (const v4l2_buffer &queryBuf, uint8_t *data, std::size_t pixelCount) :
            OffsetBasedCapturedBuffer {data, queryBuf.length, 0, pixelCount, 0},
            m_index {queryBuf.index},
            m_length {queryBuf.length}
        {
        }

        // The OffsetBasedCapturedBuffer class has already deleted the copy constructor and copy
        // assignment.

        ~CapturedV4lBuffer() override
        {
            /** Free the mmap'd memory where the data from the sensor is written */
            auto data = getUnderlyingBuffer();
            if (data)
            {
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
                LOG (DEBUG) << "munmapping buffer " << m_index;
#endif
                auto ret = munmap (data, m_length);
                if (ret == -1)
                {
                    LOG (ERROR) << "Error while munmapping buffer " << errno;
                }
            }
        }

        /**
         * The index in both m_currentBuffers[index] and v4l2_buffer.index.  Stored here instead of
         * searching m_currentBuffers for this object.
         */
        __u32 getIndex()
        {
            return m_index;
        }

    private:
        __u32 m_index;
        /** The v4l2_buffer.length value (capacity, not currently valid data) */
        __u32 m_length;
    };
}

BridgeV4l::BridgeV4l (const std::string &filename, v4l2PixelFormat format) :
    m_devFilename {filename},
    m_pixelFormat (format)
{
}

bool BridgeV4l::isConnected() const
{
    return static_cast<bool> (m_deviceHandle);
}

BridgeV4l::~BridgeV4l()
{
    if (isConnected())
    {
        try
        {
            closeConnection();
        }
        catch (...)
        {
            // ignore, there's no more cleanup possible when we can't talk to the device
        }
    }
}

std::shared_ptr<BridgeV4l::RaiiFileDescriptor> BridgeV4l::getDeviceHandle()
{
    auto handle = m_deviceHandle;
    if (!handle)
    {
        throw Disconnected();
    }

    return handle;
}

BridgeV4l::RaiiFileDescriptor::RaiiFileDescriptor (const std::string &filename) :
    fd {open (filename.c_str(), O_RDWR) }
{
    if (fd < 0)
    {
        throw CouldNotOpen ();
    }
}

BridgeV4l::RaiiFileDescriptor::~RaiiFileDescriptor()
{
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << "BridgeV4l closing fd " << fd;
#endif
    close (fd);
}

void BridgeV4l::openConnection ()
{
    if (isConnected ())
    {
        LOG (ERROR) << "bridge already connected";
        throw LogicError ("bridge already connected");
    }

#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << "Trying to open " << m_devFilename << " ... ";
#endif
    m_deviceHandle = std::make_shared<RaiiFileDescriptor> (m_devFilename);
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << "Open " << m_devFilename << " with fd " << m_deviceHandle->fd;
#endif

    // Ask for exclusive use of the device.  Not doing this allows two BridgeV4l instances to
    // successfully openConnection() on the same device, the problem is not spotted until the call
    // to createAndQueueV4lBuffers(), which is not an expected time for a CouldNotOpen to be thrown.
    v4l2_priority priority = V4L2_PRIORITY_RECORD;
    int err = ioctl (m_deviceHandle->fd, VIDIOC_S_PRIORITY, &priority);
    LOG (DEBUG) << "ioctl VIDIOC_S_PRIORITY";
    if (err)
    {
        // store the current errno, as close() may change it
        int errnum = errno;
        LOG (ERROR) << "Failed BridgeV4l::openConnection, error " << errno;
        m_deviceHandle.reset();

        switch (errnum)
        {
            case EBUSY:
                throw DeviceIsBusy ("Another application is using this camera");
            default:
                throw RuntimeError ();
        }
    }
}

void BridgeV4l::closeConnection()
{
    stopCapture();

    if (m_deviceHandle)
    {
        waitCaptureBufferDealloc();
        m_deviceHandle.reset();
    }
}

float BridgeV4l::getPeakTransferSpeed()
{
    // not yet supported
    return std::numeric_limits<float>::infinity();
}

void BridgeV4l::videoSetFormat (unsigned int imageWidth, unsigned int imageHeight)
{
    struct v4l2_format fmt;
    int ret;

    memset (&fmt, 0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = imageWidth;
    fmt.fmt.pix.height = imageHeight;
    fmt.fmt.pix.pixelformat = getV4l2Format (m_pixelFormat);
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    ret = ioctl (m_deviceHandle->fd, VIDIOC_S_FMT, &fmt);
    LOG (DEBUG) << "ioctl VIDIOC_S_FMT";
    if (ret < 0)
    {
        throw LogicError (std::string ("Could not set v4l2 format err: ") + strerror (errno));
    }

    LOG (DEBUG) << "V4L Format set ok. Width: " << fmt.fmt.pix.width
                << " Height: " << fmt.fmt.pix.height
                << " Size: " << fmt.fmt.pix.sizeimage;

    m_width = imageWidth;
    m_height = imageHeight;
}

std::size_t BridgeV4l::executeUseCase (int imageWidth, int imageHeight, std::size_t bufferCount)
{
    // There's a reliance on stopCapture() first, so that the acquisition thread isn't holding a
    // buffer.  RoyaleCore always does stop the capture, so this can be relied on.
    if (m_runAcquisition)
    {
        throw NotImplemented ("BridgeV4l only supports changing use case when stopped");
    }
    if (!isConnected())
    {
        openConnection();
    }


    bool reallocBuffers = ! (m_width == imageWidth &&
                             m_height == imageHeight &&
                             m_currentBuffers.size() == bufferCount);

    if (reallocBuffers)
    {
        LOG (DEBUG) << "Destroying buffers for new use case";
        // Wait for FrameCollector and processing to release all buffers back to the V4L layer
        waitCaptureBufferDealloc();
        // Stop acquisition thread and turn stream off
        stopCapture();
        // Release all buffers from driver
        destroyBuffers();

        videoSetFormat (imageWidth, imageHeight);
        createAndQueueV4lBuffers (bufferCount, imageWidth * imageHeight);
    }

    return m_currentBuffers.size();
}

void BridgeV4l::videoStopStream()
{
    if (!m_streamOn)
    {
        LOG (WARN) << "videoStopStream() called with stream already off";
        return;
    }

    // Calling VIDIOC_STREAMOFF will unblock the acquisition thread
    const static int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int err = ioctl (m_deviceHandle->fd, VIDIOC_STREAMOFF, &streamType);
    LOG (DEBUG) << "ioctl VIDIOC_STREAMOFF";
    switch (err)
    {
        case 0:
            m_streamOn = false;
            // no error
            break;
        case ENODEV:
        case ENXIO:
            // device disconnected
            LOG (WARN) << "Error when stopping streaming, device already disconnected " << errno;
            break;
        default:
            LOG (ERROR) << "Error when stopping streaming, error " << errno;
    }
}

void BridgeV4l::videoStartStream()
{
    if (m_streamOn)
    {
        LOG (WARN) << "videoStartStream() called with stream already on";
        return;
    }

    const static int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int err = ioctl (m_deviceHandle->fd, VIDIOC_STREAMON, &streamType);
    LOG (DEBUG) << "ioctl VIDIOC_STREAMON";
    if (err)
    {
        LOG (ERROR) << "Failed to start the streaming, error " << errno;
        throw NotImplemented ("TODO: Add error handling");
    }
    m_streamOn = true;
}


void BridgeV4l::startCapture()
{
    if (!isConnected ())
    {
        throw LogicError ("Not connected");
    }

    std::lock_guard<std::mutex> lock (m_acquisitionStartStopLock);
    videoStartStream();
    m_runAcquisition = true;
    m_acquisitionThread = std::thread (&BridgeV4l::acquisitionFunction, this);
}

void BridgeV4l::stopCapture()
{
    std::lock_guard<std::mutex> lock (m_acquisitionStartStopLock);
    bool acquisition = m_runAcquisition;
    if (acquisition)
    {
        m_runAcquisition = false;
    }
    else
    {
        LOG (DEBUG) << "m_runAcquisition already disabled";
    }

    videoStopStream();

    if (acquisition)
    {
        // even if there's an error the acquisition thread needs to be join()'d
        m_acquisitionThread.join();
    }
}

void BridgeV4l::acquisitionFunction()
{
    // If the acquisition thread encounters an unexpected error (possibly a disconnection) then the
    // thread should stop.  But leave m_runAcquisition as true so that the thread cleanup and join()
    // is still done.
    bool keepRunning = true;

    while (m_runAcquisition && keepRunning)
    {
        struct v4l2_buffer ioctlBuffer
        {
            0
        };
        ioctlBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctlBuffer.memory = V4L2_MEMORY_MMAP;

#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
        LOG (DEBUG) << "Trying to capture a frame";
#endif
        int err = ioctl (m_deviceHandle->fd, VIDIOC_DQBUF, &ioctlBuffer);
        if (err)
        {
            int errnum = errno;
            switch (errnum)
            {
                case ENODEV:
                case ENXIO:
                    // Likely a device disconnection
                    keepRunning = false;
                    break;
                default:
                    // We're probably stopping the acquisition thread after stopCapture has been called.  If
                    // it hasn't, have a short delay so that this doesn't become a busy loop.
                    if (m_runAcquisition)
                    {
                        LOG (ERROR) << "Failed to capture a video frame, error " << errnum;
                        m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING, "Failed to capture a video frame");
                        std::chrono::milliseconds delay (100);
                        std::this_thread::sleep_for (delay);
                    }
                    break;
            }
            continue;
        }
        else if (ioctlBuffer.flags & V4L2_BUF_FLAG_ERROR)
        {
            LOG (WARN) << "V4L returned success but set the V4L2_BUF_FLAG_ERROR bit";
            // This suggests data corruption but the streaming can continue. Pass the buffer on to
            // Royale; if the corruption is so severe that the pseudodata is corrupted then the
            // FrameCollector will log it, otherwise it's likely to be useful to still see data
            // arriving.
            //
            // \todo check what errors are expected to reach this, and improve the handling
        }

        if (ioctlBuffer.index >= m_currentBuffers.size())
        {
            // Should be unreachable, as m_currentBuffers is expected to be constant while the
            // acquisition thread is running.
            LOG (ERROR) << "DQBUF succeeded for buffer " << ioctlBuffer.index << " but BridgeV4l only has " << m_currentBuffers.size() << " buffers allocated";
            continue;
        }
        LOG (DEBUG) << "ioctl DQBUF(" << ioctlBuffer.index << ") succeeded";

#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
        LOG (DEBUG) << "Captured a frame in V4L buffer " << ioctlBuffer.index;
#endif
        auto *buffer = static_cast<CapturedV4lBuffer *> (m_currentBuffers[ioctlBuffer.index].get());

        // Does the data need normalisation?
        if (m_transferFormat == BufferDataFormat::UNKNOWN)
        {
            // There's sample code for auto-detecting in BridgeCopyAndNormalize, which can be used
            // here. However, it's omitted here because:
            //
            // On an embedded system, it's recommended that hardware acceleration provides the
            // buffers in the format for passing directly to IBufferCaptureListener (16-bit, native
            // endian with the top 4 bits clear), so that the BufferUtils::normalize call can be
            // omitted.
            //
            // When using BridgeUvcV4l's overridden buffer sizes, the auto-detection would need more
            // logic.  For UVC with the Arctic firmware, the buffer data is expected to already be
            // known, interoperability with V4L's UVC implementation requires v0.14.1 or later, and
            // therefore there will always be support for IDeviceStatus::getUsbTransferFormat().
            LOG (ERROR) << "BridgeV4l has code for normalizing the data format, but is set to BufferDataFormat::UNKNOWN";
        }
        else
        {
            BufferUtils::normalize (*buffer, m_transferFormat);
        }

        try
        {
            // The buffer allocation does not change while the acquisitionFunction is running, so
            // most of this function is implicitly synchronized.  However this mutex is needed for
            // changing the m_listenerBufferCount.
            //
            // There is almost always going to be a listener, so there's no optimization of the
            // path where queueBuffer is called a couple of lines below this.
            {
                std::lock_guard<std::mutex> lock (m_bufferLock);
                m_listenerBufferCount++;
            }

            std::lock_guard<std::mutex> lock (m_changeListenerLock);
            if (m_captureListener)
            {
                m_captureListener->bufferCallback (buffer);
            }
            else
            {
                queueBuffer (buffer);
            }
        }
        catch (const Exception &e)
        {
            // bufferCallback() can fail intermittently due to truncated buffers delivered by the system.
            // As this is a temporary issue and capturing continues afterwards, it's EVENT_SEVERITY_WARNING only.
            m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING,
                    e.getTechnicalDescription());
        }
    }
}

void BridgeV4l::createAndQueueV4lBuffers (std::size_t bufferCount, std::size_t pixelCount)
{
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << "Allocating " << bufferCount << " buffers";
#endif

    std::lock_guard<std::mutex> lock (m_bufferLock);
    if (!m_deviceHandle)
    {
        LOG (ERROR) << "Video device not opened";
        throw LogicError ("Video device not opened");
    }

    if (!m_currentBuffers.empty() || m_bufferCount != 0)
    {
        LOG (ERROR) << "createAndQueueV4lBuffers called with already-allocated buffers";
        throw LogicError ("createAndQueueV4lBuffers called with already-allocated buffers");
    }

    int err = 0;
    struct v4l2_requestbuffers req
    {
        0
    };
    req.count = static_cast<__u32> (bufferCount);
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    err = ioctl (m_deviceHandle->fd, VIDIOC_REQBUFS, &req);
    LOG (DEBUG) << "ioctl VIDIOC_REQBUFS(" << bufferCount << ")";
    if (err)
    {
        LOG (ERROR) << "Failed to allocate video buffers, error " << errno;
        if (errno == EBUSY)
        {
            throw CouldNotOpen ("Device busy");
        }
        throw NotImplemented ("TODO: Add error handling");
    }

#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << "Driver allocated " << req.count << " V4L buffers, type " << req.type;
#endif
    m_bufferCount = static_cast<std::size_t> (req.count);

    std::vector<std::unique_ptr<royale::hal::ICapturedBuffer>> handles;
    for (std::size_t i = 0; i < m_bufferCount; i++)
    {
        struct v4l2_buffer queryBuf
        {
            0
        };

        queryBuf.index = static_cast<__u32> (i);
        queryBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        queryBuf.memory = V4L2_MEMORY_MMAP;

        err = ioctl (m_deviceHandle->fd, VIDIOC_QUERYBUF, &queryBuf);
        LOG (DEBUG) << "ioctl VIDIOC_QUERYBUF";
        if (err)
        {
            LOG (ERROR) << "Failed to allocate video buffer number " << i << ", error " << errno;
            throw NotImplemented ("TODO: Add error handling");
        }

        // The buffer length is chosen by the driver, probably from a configuration provided with
        // the camera.  For V4L's UVC driver, it's based on the UVC dwMaxVideoFrameSize, which in
        // Arctic firmware is larger than any expected superframe size.
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
        LOG (DEBUG) << "Going to mmap video buffer number " << i << ", length " << queryBuf.length;
#endif
        if (queryBuf.length < pixelCount * sizeof (uint16_t))
        {
            LOG (ERROR) << "Buffer is too small to handle the expected image size";
            throw NotImplemented ("TODO: configure larger buffers in the driver");
        }
        // mmap only the size that's needed, assuming the driver does not use 24 bits per pixel.
        queryBuf.length = static_cast<__u32> (pixelCount * sizeof (uint16_t));

        // Here PROT_WRITE is requested because of the in-place data normalization in the
        // acquisitionFunction. If the driver is providing data that's already in the format for
        // ICapturedBuffer then PROT_READ should be sufficient.
        uint8_t *data = static_cast<uint8_t *> (mmap (NULL, queryBuf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_deviceHandle->fd, queryBuf.m.offset));
        if (MAP_FAILED == data)
        {
            LOG (ERROR) << "Failed to mmap video buffer number " << i << ", error " << errno;
            throw NotImplemented ("TODO: Add error handling");
        }

        auto captureBuffer = common::makeUnique<CapturedV4lBuffer> (queryBuf, data, pixelCount);
        handles.push_back (std::move (captureBuffer));
    }

    swap (m_currentBuffers, handles);

    for (auto &frame : m_currentBuffers)
    {
        queueBufferLocked (frame.get());
    }
}

void BridgeV4l::setBufferCaptureListener (royale::hal::IBufferCaptureListener *listener)
{
    std::lock_guard<std::mutex> lock (m_changeListenerLock);
    m_captureListener = listener;
}

void BridgeV4l::setEventListener (royale::IEventListener *listener)
{
    m_eventForwarder.setEventListener (listener);
}

void BridgeV4l::setTransferFormat (royale::buffer::BufferDataFormat format)
{
    m_transferFormat = format;
}

void BridgeV4l::queueBuffer (royale::hal::ICapturedBuffer *buffer)
{
    std::lock_guard<std::mutex> lock (m_bufferLock);
    m_listenerBufferCount--;
    queueBufferLocked (buffer);
}

void BridgeV4l::queueBufferLocked (royale::hal::ICapturedBuffer *cap)
{
    // Error handling in this function would have to depend on what the cause of the error is, at
    // the moment it simply logs and returns without queueing the buffer.
    //
    // Although it's not documented whether queueBuffer() can throw, it probably shouldn't except
    // for cases that would noticed after very minimal testing.
    CapturedV4lBuffer *buffer = dynamic_cast<CapturedV4lBuffer *> (cap);
    if (buffer == nullptr)
    {
        LOG (ERROR) << "Wrong type of buffer for this bridge";
        return;
    }

    if (m_bufferChangeInProgress)
    {
        tryDeallocBuffersLocked();
        return;
    }

    struct v4l2_buffer ioctlBuffer
    {
        0
    };
    ioctlBuffer.index = buffer->getIndex();
    ioctlBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctlBuffer.memory = V4L2_MEMORY_MMAP;
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << "Queuing video buffer " << ioctlBuffer.index;
#endif
    auto err = ioctl (m_deviceHandle->fd, VIDIOC_QBUF, &ioctlBuffer);
    LOG (DEBUG) << "ioctl VIDIOC_QBUF(" << ioctlBuffer.index << ")";
    switch (err)
    {
        case 0:
            // no error
            break;
        case ENODEV:
        case ENXIO:
            // device disconnected
            LOG (WARN) << "Error when stopping streaming, device already disconnected " << errno;
            break;
        default:
            LOG (ERROR) << "Error when stopping streaming, error " << errno;
    }
    if (err)
    {
        LOG (ERROR) << "Failed to queue video buffer, error " << errno;
    }
}

void BridgeV4l::destroyBuffers (void)
{
    int err = 0;
    std::lock_guard<std::mutex> lock (m_bufferLock);
    if (m_bufferCount == 0)
    {
        return;
    }

    struct v4l2_requestbuffers req
    {
        0
    };
    req.count = 0;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    err = ioctl (m_deviceHandle->fd, VIDIOC_REQBUFS, &req);
    LOG (DEBUG) << "ioctl VIDIOC_REQBUFS(0)";
    if (err)
    {
        LOG (ERROR) << "Failed to destroy video buffers, error " << errno;
        if (errno == EBUSY)
        {
            throw CouldNotOpen ("Device busy");
        }
        throw NotImplemented ("TODO: Add error handling");
    }
    m_bufferCount = static_cast<std::size_t> (req.count);
}

void BridgeV4l::waitCaptureBufferDealloc()
{
    if (m_runAcquisition)
    {
        throw LogicError ("waiting for buffer deallocation while the acquisition thread is running");
    }
    // Scopes for two different locks, to call releaseAllBuffers() in a state where queueBuffer
    // won't block.  This shouldn't be necessary (the framecollectors are supposed to release the
    // buffers before and during executeUseCase), but the listener is not necessarily a
    // framecollector.
    {
        std::lock_guard<std::mutex> lock (m_bufferLock);
        m_bufferChangeInProgress = true;
    }
    {
        std::lock_guard<std::mutex> lock (m_changeListenerLock);
        if (m_captureListener)
        {
            m_captureListener->releaseAllBuffers();
        }
    }

    std::unique_lock<std::mutex> lock (m_bufferLock);
    tryDeallocBuffersLocked();
    m_sleepCV.wait (lock, [this] { return !m_bufferChangeInProgress; });
    LOG (DEBUG) << "All buffers unmapped";
}

void BridgeV4l::tryDeallocBuffersLocked()
{
#ifdef ROYALE_LOGGING_VERBOSE_BRIDGE
    LOG (DEBUG) << __PRETTY_FUNCTION__ << ": " << m_listenerBufferCount << "/" << m_currentBuffers.size() << " buffers still in the listener";
#endif
    if (m_listenerBufferCount > 0)
    {
        return;
    }

    m_currentBuffers.clear();
    m_bufferChangeInProgress = false;
    m_sleepCV.notify_all();
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeV4l::getBridgeInfo()
{
    auto info = royale::Vector<royale::Pair<royale::String, royale::String>> {};
    info.emplace_back ("BRIDGE_TYPE", "V4L");
    return info;
}
