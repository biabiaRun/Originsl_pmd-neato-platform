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
    namespace hal
    {
        /**
         * This is a callback to the IBridgeDataReceiver from the Royale Core.
         */
        class IBufferCaptureReleaser
        {
        public:
            virtual ~IBufferCaptureReleaser() = default;

            /**
             * Will be called on all CapturedBuffers that are passed to
             * IBufferCaptureListener::bufferCallback.  This returns buffer ownership to the HAL
             * module, and allows the memory to be reused.
             *
             * It might be called in any thread, even in the IBridgeDataReceiver's own thread during
             * the call to IBufferCaptureListener::bufferCallback.
             */
            virtual void queueBuffer (royale::hal::ICapturedBuffer *buffer) = 0;
        };
    }
}
