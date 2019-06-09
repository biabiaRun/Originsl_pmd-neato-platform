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


namespace royale
{
    namespace collector
    {
        /**
         * This is a callback to the Bridge from the processing chain.
         */
        class IFrameCaptureReleaser
        {
        public:
            virtual ~IFrameCaptureReleaser() = default;

            /**
             * Must be called on all CapturedRawFrames that are provided by the CaptureListener
             * interface.  This returns buffer ownership to the access module, and allows the
             * memory to be reused.
             *
             * Although both CaptureListener and CaptureReleaser use vector<ICapturedRawFrame *>, it
             * is not necessary for the frames to be released in the same order, or the same groups,
             * as the calls to CaptureListener.  A vector is used because the CaptureListeners often
             * release all frames at the same time, and also to allow optimizion of the
             * CaptureReleaser, which may need to acquire a lock for each call to this function.
             *
             * This may be called from any thread.
             */
            virtual void releaseCapturedFrames (std::vector<common::ICapturedRawFrame *> frames) = 0;
        };
    }
}
