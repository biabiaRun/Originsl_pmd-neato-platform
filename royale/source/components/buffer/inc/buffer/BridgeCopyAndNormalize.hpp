/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <buffer/BridgeInternalBufferAlloc.hpp>
#include <buffer/BufferDataFormat.hpp>

#include <common/EventForwarder.hpp>

#include <atomic>

namespace royale
{
    namespace buffer
    {
        /**
         * Common code for an IBridgeDataReceiver that has to normalize the data buffers that it
         * receives, and which copies the buffers at the same time as normalizing them.
         *
         * The supported formats are the ones defined in the BufferDataFormat enum, they will be
         * converted to Royale's internal format.
         *
         * Additionally, this Bridge supports format autodetection; if the data format is set to
         * BufferDataFormat::UNKNOWN then it will try to detect it from the incoming data.
         * Currently the auto-detection is simply based on comparing the data size to the expected
         * sizes for each format.  Once the format has been auto-detected, the bridge will keep
         * using the same format, so that a mixed-mode with variable-size superframes doesn't end up
         * with one size being correctly detected, and the other size incorrectly detected.
         *
         * See bridgeAcquisitionCallback's documentation for the behavior of startCapture and
         * stopCapture in this class.
         */
        class BridgeCopyAndNormalize : public BridgeInternalBufferAlloc
        {
        public:
            /**
             * Constructor
             *
             * @param format see setTransferFormat().
             */
            ROYALE_API explicit BridgeCopyAndNormalize (BufferDataFormat format = BufferDataFormat::UNKNOWN);
            ROYALE_API ~BridgeCopyAndNormalize() override;

            // From BridgeInternalBufferAlloc (IBridgeDataReceiver and IBufferCaptureReleaser)
            std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;
            void startCapture() override;
            void stopCapture() override;
            royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;
            void setEventListener (royale::IEventListener *listener) override;
            // From BridgeInternalBufferAlloc itself
            bool shouldBlockForDequeue() override;

            /**
             * Set the format that buffers are received in.  This is only expected to be called
             * during bridge creation before image data is received (for example by the
             * BridgeFactory), it's provided for bridges that have to be constructed and query their
             * device before they know which data format will be used.
             *
             * If it should be called later in the Bridge lifecycle, then it will need thread safety
             * to be added.
             *
             * BufferDataFormat::UNKNOWN enables auto-detection of the format.
             */
            void setTransferFormat (BufferDataFormat format);

            /**
             * If the format has been set or auto-detected, returns the format.  If this class is
             * waiting for data so that it can be auto-detected, returns UNKNOWN.
             *
             * This is intended for implementation of getPeakTransferSpeed().
             */
            BufferDataFormat getTransferFormat();

        protected:
            /**
             * Copy the data to an internally-managed buffer, and send that buffer to the
             * IBufferCaptureListener.  This does not block; if no buffers are available, or if
             * stopCapture() has been called more recently than startCapture(), it will simply
             * return, dropping the data.
             *
             * The original data is copied, and will be ready to reuse or deallocate as soon as this
             * function returns.
             *
             * The caller must not hold any lock when sending buffers to the BufferCaptureListener
             * (or at least, must not hold any lock that could block that thread from calling any
             * Bridge, Imager or CameraModule function).
             *
             * If the last call to setBufferCaptureListener set the listener to nullptr, this will
             * simply call queueBuffer.
             *
             * If startCapture has not been called, or has not been called again after the most
             * recent call to stopCapture, this will simply call requeue the buffers.  This is to
             * support some subclasses where the hardware capture starts when the bridge is
             * connected and stops only when it's disconnected.
             */
            void bridgeAcquisitionCallback (
                const size_t sampleSize,
                const void *data,
                uint64_t timestamp);

        private:
            /**
             * Whether the data received will be in RAW12 or RAW16 format, or auto-detect.
             */
            BufferDataFormat m_transferFormat;

            /**
             * False if stopCapture() has been called more recently than startCapture(), and also
             * false if startCapture() has never been called.  If frames are received while this is
             * false, they'll be dropped immediately.
             */
            std::atomic<bool> m_captureStarted;

            /**
             * Event forwarder.
             */
            royale::EventForwarder m_eventForwarder;
        };
    }
}
