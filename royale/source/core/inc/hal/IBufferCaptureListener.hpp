/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <hal/ICapturedBuffer.hpp>

namespace royale
{
    /* The implementation of this HAL class is provided by Royale Core, system integrators do not
     * need to provide an implementation.
     */
    namespace hal
    {
        /**
         * This interface is how the IBridgeDataReceiver provides images to be processed by the
         * Royale Core.  It receives the captured data from the IBridgeDataReceiver, combines it
         * with knowledge of the imager's data format, and calculates which groups of
         * CapturedBuffers should be combined to form a depth image.
         *
         * Although IBufferCaptureListener is part of the set of HAL interfaces, the implementation
         * is provided by Royale Core.  System integrators do not need to implement this, the Core
         * will pass it to IBridgeDataReceiver::setBufferCaptureListener()
         */
        class IBufferCaptureListener
        {
        public:
            IBufferCaptureListener() = default;
            IBufferCaptureListener (const IBufferCaptureListener &) = delete;
            IBufferCaptureListener &operator= (const IBufferCaptureListener &) = delete;

            virtual ~IBufferCaptureListener() = default;

            /**
             * Called from the IBridgeDataReceiver when a buffer has been captured.  This takes
             * ownership of the buffer, and will eventually call IBufferCaptureReleaser::queueBuffer
             * to return ownership.  Please note that the queueBuffer may happen at any time, and in
             * any thread, and it may happen before the call to bufferCallback returns.
             *
             * The caller must not hold any lock while calling this function (or at least, must not
             * hold any lock that could block any Bridge, Imager or CameraModule function).
             *
             * The data must already be in normalized form (16-bit native endian, with the top 4
             * bits clear).
             */
            virtual void bufferCallback (royale::hal::ICapturedBuffer *buffer) = 0;

            /**
             * This cleanup function can be called before the IBufferCaptureReleaser is deleted,
             * and will block until all CapturedBuffers have been returned to their Releaser.  The
             * Listener will not make any calls to the Releaser after this function returns, unless
             * the Bridge sends more buffers to Listener.
             *
             * Most Bridges should not need to call this function when running with the Royale Core,
             * but unit-tests, unusual Bridges and the v1 Royale bridge framework may need it.
             */
            virtual void releaseAllBuffers() = 0;
        };
    }
}
