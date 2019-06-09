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

#include <common/ICapturedRawFrame.hpp>
#include <collector/CapturedUseCase.hpp>
#include <usecase/UseCaseDefinition.hpp>

#include <memory>
#include <vector>

namespace royale
{
    namespace collector
    {
        /**
         * These are calls from the Royale Core to the processing chain.
         *
         * Note: deleting the camera from within either callback is
         * not supported.
         */
        class IFrameCaptureListener
        {
        public:
            virtual ~IFrameCaptureListener() = default;

            /**
             * Callback when a complete UseCaseDefinition of phase images has been received.
             *
             * This will only be called by one thread. If the processing is multithreaded, it must
             * pass the captured frames to a separate thread and return from this function, there
             * will be no further callbacks until this function returs.
             *
             * The listener must call CaptureReleaser::releaseCapturedFrames() at some point, and if
             * it can do so before calling its own listener then it should, to release the buffers
             * earlier.
             *
             * The CapturedUseCase may contain pointers to data with a different lifecycle.  Those
             * pointers are guaranteed to be valid until all of the CapturedRawFrames that
             * accompanied the CapturedUseCase have been released.  After releasing the frames, the
             * listener should not access the CapturedUseCase again (except to destroy it).
             *
             * Ownership of the buffers passes to the listener, and they must be requeued
             * afterwards for reuse, by calling CaptureReleaser::releaseCapturedFrames().
             * The caller will ignore the contents of the captured vector after this function
             * returns; leaving pointers in the vector will not cause the pointed-to
             * CapturedRawFrames to be requeued.
             *
             * The releaseCapturedFrames() function can be called from any thread, and the listener
             * may store the frames for processing in another thread.
             */
            virtual void captureCallback (std::vector<common::ICapturedRawFrame *> &frames,
                                          const royale::usecase::UseCaseDefinition &definition,
                                          royale::StreamId streamId,
                                          std::unique_ptr<const CapturedUseCase> capturedCase) = 0;

            /**
             * This cleanup function must block until all the ICapturedRawFrames that were passed to
             * the captureCallback have been returned by calling releaseCapturedFrame().
             *
             * A FrameCaptureListener which only uses the thread that calls captureCallback, and
             * always releases all frames before returning from the captureCallback, can simply
             * implement this function with an empty method body.
             *
             * The caller will handle locking so that there are never simultaneous calls to
             * captureCallback and releaseAllFrames.
             */
            virtual void releaseAllFrames() = 0;
        };
    }
}
