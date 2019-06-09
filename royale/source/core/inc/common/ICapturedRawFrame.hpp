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

#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * This contains the data from one Raw Frame of the imager.
         *
         * Although this has a virtual destructor, it should only be released by calling
         * CaptureReleaser::releaseCapturedFrames().  It is expected to be subclassed by the
         * IFrameCollector subclasses, to fit with the memory-management of the framework that is
         * used for data capture.
         */
        class ICapturedRawFrame
        {
        public:
            virtual ~ICapturedRawFrame() = default;

            /**
             * The optical data seen by the imager, in normalised 16-bit, native-endian format.
             *
             * The pointer is valid until CaptureReleaser::releaseCapturedFrames() is called.
             */
            virtual uint16_t *getImageData() = 0;

            /**
             * Returns the metadata of this frame, this is an opaque data structure which should
             * only be accessed via the module's PseudoDataInterpreter.
             *
             * This is valid even on devices that don't include pseudodata in the data from the
             * hardware.  On those devices, it will contain data generated in the Bridge.
             *
             * The pointer is valid until CaptureReleaser::releaseCapturedFrames() is called.
             */
            virtual const uint16_t *getPseudoData() const = 0;
        };
    }
}
