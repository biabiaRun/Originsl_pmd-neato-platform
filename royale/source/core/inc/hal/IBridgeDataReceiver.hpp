/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <hal/IBufferCaptureListener.hpp>
#include <hal/IBufferCaptureReleaser.hpp>
#include <royale/IEventListener.hpp>
#include <royale/String.hpp>
#include <royale/Vector.hpp>
#include <royale/Pair.hpp>

namespace royale
{
    namespace hal
    {
        /**
         * Interface of Bridge functions for reading from the main data-capture source.
         * For example, in a MIPI CSI-2 implementation, this would access the CSI-2 Receiver.
         *
         * An implementation of this class is expected to read data from the platform.  On platforms
         * where the system expects the application to make a blocking or polling read of the data,
         * the implementation should create its own internal thread.  The data then passes to the
         * IBufferCaptureListener (also called the FrameCollector), which returns the buffers
         * asynchronously via IBufferCaptureReleaser.
         *
         * The sensor itself is controlled by IBridgeImager, an implementation may choose to use a
         * single class to implement both interfaces.
         */
        class IBridgeDataReceiver : public IBufferCaptureReleaser
        {
        public:
            ROYALE_API virtual ~IBridgeDataReceiver() = default;

            /**
             * Each received data buffer should be passed to this listener.  This will be called
             * before the first call to executeUseCase.
             *
             * If the listener has already been set, a second call to setBufferCaptureListener
             * replaces the old listener - the IBridgeDataReceiver must not send buffers to the old
             * listener, which may be deleted as soon as this call returns.
             *
             * During the shutdown sequence, this will be called with nullptr, after which the old
             * listener may be deleted.
             */
            ROYALE_API virtual void setBufferCaptureListener (IBufferCaptureListener *collector) = 0;

            /**
             * Called after sending commands to the imager that may change the buffer size, or that
             * may change the expected number of buffers required.  It should create buffers for
             * capturing images with the new settings.
             *
             * This may block until all previously-allocated buffers have been requeued.  It should
             * not pass more buffers to bufferCallback while blocking waiting for the previous
             * buffers to be requeued.
             *
             * This will only be called while the capture is stopped; if necessary, the CameraCore
             * will call stopCapture() before executeUseCase().  However, it may be called while
             * there are still buffers in the IBufferCaptureListener.
             *
             * Note: the previous paragraph has been true since Royale's v2 architecture, however it
             * was explicitly documented here that the bridge must expect executeUseCase() during
             * capturing.  Many of the reference bridge implementations either have support for that
             * or an explicit sanity check for it, but these are obsolete.
             *
             * One of the arguments is the preferredBufferCount, which will change depending on the
             * use case.  This number will allow the buffers to be passed to the processing chain
             * without needing to be copied, and depends on superframes, double buffering, whether
             * subsets of the full UseCaseDefinition can be passed to the processing chain, etc.
             *
             * The preferredBufferCount should be treated as the minimum number of buffers required,
             * and the IBridgeDataReceiver should be able to send this many ICapturedBuffers to the
             * listener without needing to wait for buffers to be released.  It's assumed that
             * platform integrators will check this during development; if the platform won't allow
             * the expected numbers of buffers to be allocated then the bridge implementation must
             * allocate buffers itself, BridgeInternalBufferAlloc is a utility class for this.
             *
             * \return the number of buffers which were successfully allocated
             */
            ROYALE_API virtual std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) = 0;

            /**
             * Return the approximate data rate that the hardware bridge can support for reading
             * the image data.  This is the theoretical bandwidth, the caller should interpret it
             * as the peak speed over short periods of time, milliseconds or shorter, taking account
             * of the peak-and-stop nature of the data provided by image capture.
             *
             * Bridges that can not determine the correct value return infinity.  This is expected
             * to be in the "fastest" category for any algorithm that simply wants to check for
             * "fast enough", and be recognised as a sentinel value by any algorithm that is
             * actually calculating based on the number.
             *
             * This can be implemented using constant values for the theoretical maximum, without
             * needing to measure the actual value.
             *
             * Examples:
             *
             * A MIPI CSI-2 Bridge might just have constants for 1-lane or 2-lane operation
             *
             * A USB-based Bridge would need to check if its connection was "High Speed" or
             * "SuperSpeed", but can then just map the USB library's connection-type enum to
             * constants, etc.
             *
             * \return approximate speed in pixels per second (not bytes per second)
             */
            ROYALE_API virtual float getPeakTransferSpeed() = 0;

            /**
             * Activate the data-receiving thread and hardware, if that is necessary for this
             * platform.  May be a no-op on some platforms.
             *
             * This does not trigger the Imager to start sending data, it only triggers the Bridge
             * to read the data.
             *
             * This will only be called while the capture is stopped.
             */
            ROYALE_API virtual void startCapture() = 0;

            /**
             * After this function is called, no more frames should be sent to the
             * IBufferCaptureListener until startCapture() is called again.
             *
             * The bridge implementation may power down the receiving hardware, or it may merely
             * set a flag to discard any received data until the next call to startCapture().
             *
             * This may be called multiple times, even when the capture has already stopped.
             *
             * Before destroying the IBufferCaptureListener, both stopCapture() and
             * then setBufferCaptureListener(nullptr) will be called.
             */
            ROYALE_API virtual void stopCapture() = 0;

            /**
             * Connection status of the bridge.
             * Should never returns false if connected, may also return true, even if the bridge is
             * disconnected due to system implementation differences.
             *
             * \return connection status of the bridge
             */
            ROYALE_API virtual bool isConnected() const = 0;

            /**
            * Retrieve extended information about the module. This depends on the implementation
            * of the bridge that is used.
            *
            * \return A vector of string pairs that gives keys with corresponding values
            */
            ROYALE_API virtual royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() = 0;

            /**
            * Set event listener the bridge should use for asynchronous error reporting.
            *
            * The CameraCore will call this function to register a valid listener during initialization
            * and to deregister it (by passing a nullptr) during shutdown.
            * The listener passed by CameraCore only enqueues the event, it does not block for the
            * actual delivery. The CameraCore uses an EventQueue instance for decoupling, so bridge
            * implementations may generate events directly in their data acquisition threads.
            *
            * Implementations are encouraged to delegate this function to a royale::EventForwarder
            * member (which handles concurrency) and use this instead of dealing with the IEventListener directly.
            * Existing bridge implementations use royale::event::CaptureStreamEvent with an EventSeverity
            * of (at least) EVENT_SEVERITY_ERROR to signal that streaming was stopped due to errors.
            * A lower severity is used for errors that don't stop streaming (e.g. dropped frames).
            *
            * Note that even if the bridge has signalled that the stream was terminated, users are still expected
            * to call stopCapture().
            *
            * \param listener Pointer to a valid IEventListener, or nullptr to invalidate a previously set listener.
            *
            */
            ROYALE_API virtual void setEventListener (royale::IEventListener *listener) = 0;

        };
    }
}
