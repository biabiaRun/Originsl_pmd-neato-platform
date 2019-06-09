/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <buffer/BufferDataFormat.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <hal/ICapturedBuffer.hpp>

#include <common/EventForwarder.hpp>

#include <royalev4l/PixelFormat.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace royale
{
    namespace v4l
    {
        namespace bridge
        {
            /**
             * Support for cameras via the Video For Linux framework.
             *
             * This class is intended to be a generic IBridgeDataReceiver that supports all V4L
             * based devices.  The IBridgeImager and peripherals need separate support, for example
             * the UVC-specific support in BridgeUvcV4lUvcExtension.
             *
             * For this Bridge, the buffers are only allocated or deallocated when the acquisition
             * function is not running. This means that the code can assume that the buffer count
             * doesn't change while the acquisition thread is running, even without locking.
             * However, the driver does have to handle VIDIOC_STREAMOFF being used in a different
             * thread to interrupt the blocking VIDIOC_DQBUF call from the acquisition thread.
             */
            class BridgeV4l : public royale::hal::IBridgeDataReceiver
            {
            public:
                /**
                 * The filename is expected to be a /dev/video node.
                 */
                explicit BridgeV4l (const std::string &filename, v4l2PixelFormat format);
                ~BridgeV4l() override;

                void openConnection();
                /**
                 * This is virtual to enable the UVC workaround in BridgeUvcV4l::stopCapture().
                 */
                virtual void closeConnection();

                // From IBridgeDataReceiver and IBufferCaptureReleaser
                void setBufferCaptureListener (royale::hal::IBufferCaptureListener *listener) override;
                std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;
                float getPeakTransferSpeed () override;
                void startCapture() override;
                void stopCapture() override;
                royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;
                void setEventListener (royale::IEventListener *listener) override;
                void queueBuffer (royale::hal::ICapturedBuffer *buffer) override;
                bool isConnected() const override;

                /**
                 * Wrapper for a file descriptor.
                 *
                 * This is so that getDeviceHandle can return a shared_ptr, and the file descriptor
                 * will not be closed while that shared_ptr exists, prevent a race condition with
                 * the descriptor being reused for a different file.  If the device is unplugged
                 * that descriptor may become invalid, but it won't be reassigned to another valid
                 * device.
                 */
                struct RaiiFileDescriptor
                {
                    RaiiFileDescriptor (const std::string &filename);
                    RaiiFileDescriptor (const RaiiFileDescriptor &) = delete;
                    RaiiFileDescriptor (RaiiFileDescriptor &&) = delete;
                    ~RaiiFileDescriptor();
                    const int fd;
                };

                /**
                 * For implementing the control channel, this returns the file descriptor number
                 * for the V4L /dev/video... file.
                 *
                 * \throws Disconnected if closeConnection completed before this function was called
                 */
                std::shared_ptr<RaiiFileDescriptor> getDeviceHandle();

                /**
                 * Sets the format that buffers are received in.  This is only expected to be called
                 * during bridge creation before image data is received (for example by the
                 * BridgeFactory), or for the data normalization to be handled by hardware instead
                 * (see the .cpp file's comments about the m_transferFormat).
                 *
                 * BufferDataFormat::UNKNOWN will print an error message on each buffer capture, but
                 * will still pass the data through to the capture listener.
                 */
                void setTransferFormat (royale::buffer::BufferDataFormat format);

            private:
                /**
                 * Request the driver to allocate bufferCount buffers (by the V4L spec, the number
                 * of buffers allocated may be different from the number requested).  Creates
                 * entries in m_currentBuffers corresponding to the driver-allocated buffers, and
                 * queues them for capturing.
                 *
                 * Must not be called when m_currentBuffers is non-empty. After a call to this,
                 * there should be a call to waitCaptureBufferDealloc() before calling this again.
                 */
                void createAndQueueV4lBuffers (std::size_t bufferCount, std::size_t pixelCount);

                /**
                 * Triggers the release (munmap) of the buffers, and blocks until all buffers have
                 * been released.  This may require waiting for buffers to be returned by the buffer
                 * capture listener, in which case the the thread will be woken by the final call
                 * from the buffer capture listener to queueBuffer().
                 *
                 * Must not be called when capturing. The caller must ensure that stopCapture() is
                 * called first (or that startCapture() has never been called).
                 */
                void waitCaptureBufferDealloc();

                /**
                 * Internal implementation of queueBuffer, called with m_bufferLock already held.
                 *
                 * This does not change the m_listenerBufferCount, as it is also used for enqueuing
                 * the buffers during createAndQueueV4lBuffers.
                 */
                void queueBufferLocked (royale::hal::ICapturedBuffer *buffer);

                /**
                 * Checks if all buffers are currently queued (from the perspective of this bridge,
                 * not queued in the V4L driver), and if so deletes all of them and wakes any thread
                 * that is blocked in waitCaptureBufferDealloc().
                 *
                 * Only called with m_bufferLock already held, the capture already stopped, and
                 * m_bufferChangeInProgress true.
                 */
                void tryDeallocBuffersLocked();

                /**
                 * Deinitializes buffers registered in video device. Upon successful return
                 * m_bufferCount is zero.
                 */
                void destroyBuffers (void);

                /**
                 * Set's stream format for video device according to resolution parameters
                 * and m_pixelFormat.
                 */
                void videoSetFormat (unsigned int imageWidth, unsigned int imageHeight);

                /**
                 * Makes an VIDIOC_STREAMOFF ioctl for video device.
                 * Must be called with m_acquisitionStartStopLock lock held.
                 */
                void videoStopStream (void);

                /**
                 * Makes an VIDIOC_STREAMON ioctl for video device.
                 * Must be called with m_acquisitionStartStopLock lock held
                 */
                void videoStartStream (void);

                /**
                 * All buffers allocated by the last call to createAndQueueV4lBuffers, whether
                 * queued or not.
                 *
                 * Synchronization: the buffer allocation is not changed while the acquisition
                 * thread is running.
                 */
                std::vector<std::unique_ptr<royale::hal::ICapturedBuffer>> m_currentBuffers;

                /**
                 * Number of buffers which have been passed to the capture listener and not yet
                 * returned via queueBuffer. Must only be accessed with m_bufferLock held.
                 */
                std::size_t m_listenerBufferCount {0};

                /**
                 * Number of buffers currently registired in the driver.
                 */
                std::size_t m_bufferCount {0};

                /**
                 * Set from waitCaptureBufferDealloc until the buffers are deallocated.
                 *
                 * Must only be accessed with m_bufferLock held.
                 */
                bool m_bufferChangeInProgress {false};
                /**
                 * Must be held when adding or removing buffers from the vectors.
                 */
                std::mutex m_bufferLock;
                /**
                 * waitCaptureBufferDealloc sleeps on this, tryDeallocBuffersLocked notifies on it
                 * when the last buffer has been released.
                 */
                std::condition_variable m_sleepCV;

                /**
                 * File descriptor of the V4L /dev/filename
                 *
                 * When disconnected, this is empty.
                 */
                std::shared_ptr<RaiiFileDescriptor> m_deviceHandle;

                /**
                 * The V4L file that is opened as m_deviceHandle.
                 */
                std::string m_devFilename;

                /**
                 * Pixel format that will be used to initialize V4L2 device
                 */
                v4l2PixelFormat m_pixelFormat;

                /**
                 * Resolution of stream currently set in V4L2 device.
                 */
                int m_width {0};
                int m_height {0};

                /** Indicates current state of VIDIOC_STREAMOFF/VIDIOC_STREAMON operations
                 * on video device.
                 */
                bool m_streamOn {false};

                /**
                 * Thread that reads image data from the module.
                 */
                std::thread m_acquisitionThread;

                /**
                 * Main loop of m_acquisitionThread
                 */
                void acquisitionFunction();

                /**
                 * Control variable, set to false to signal m_acquisitionThread to finish.
                 */
                std::atomic<bool> m_runAcquisition {false};

                /**
                 * Lock for starting / stopping the acquisition thread. This is to prevent the
                 * situation that stopThread() followed by startThread() resets the m_runAcquisition
                 * variable before the original acquisition thread has finished.
                 *
                 * The m_acquisitionThread itself never locks this.
                 */
                std::mutex m_acquisitionStartStopLock;

                /**
                 * Must be held when accessing m_captureListener.
                 */
                std::mutex m_changeListenerLock;

                /**
                 * Where to send the buffers to
                 */
                royale::hal::IBufferCaptureListener *m_captureListener {nullptr};

                /**
                 * Whether the data received will be in RAW12 or RAW16 format.
                 *
                 * The behavior for UNKNOWN is implementation-defined, it currently passes the data
                 * through unchanged.
                 */
                royale::buffer::BufferDataFormat m_transferFormat {royale::buffer::BufferDataFormat::UNKNOWN};

                royale::EventForwarder m_eventForwarder;
            };
        }
    }
}
