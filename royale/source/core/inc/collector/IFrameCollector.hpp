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

#include <hal/ITemperatureSensor.hpp>
#include <hal/IPsdTemperatureSensor.hpp>

#include <collector/IFrameCaptureListener.hpp>
#include <collector/IFrameCaptureReleaser.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <hal/IBufferCaptureListener.hpp>
#include <hal/IBufferCaptureReleaser.hpp>
#include <royale/Definitions.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <royale/IEventListener.hpp>
#include <royale/Vector.hpp>

#include <memory>

namespace royale
{
    namespace collector
    {
        /**
         * The IBufferCaptureListener superinterface is how the IBridgeDataReceiver provides images
         * to be processed by the Royale Core.  This interface provides the other methods that are
         * called internally from the Royale Core, and the frame-releaser to release buffers after
         * they have been processed.
         *
         * The rest of this comment is about the internal buffer/frame logic.
         *
         * Some Bridges receive each frame of a UseCaseDefinition as a separate image, but some
         * receive them from the hardware as superframes.  With the appropriate subclass of
         * IFrameCollector, the hardware superframes can be passed to bufferCallback() and then cut
         * in to individual frames.
         *
         * Some UseCases may require that frames are sent to the CaptureCallback before the complete
         * UseCaseDefinition has been received, sending a subset of the frames instead of collecting
         * a full UseCaseDefinition of frames.  The appropriate IFrameCollector subclass will also
         * handle this.
         */
        class IFrameCollector : public royale::hal::IBufferCaptureListener,
            public royale::collector::IFrameCaptureReleaser
        {
        public:
            IFrameCollector() = default;
            IFrameCollector (const IFrameCollector &) = delete;
            IFrameCollector &operator= (const IFrameCollector &) = delete;

            virtual ~IFrameCollector() = default;

            /**
             * The callback to the processing pipeline, called when a complete set of frames is captured.
             *
             * If changing from one listener to another listener, note that releaseAllBuffers() will
             * only pass through to the current listener's releaseAllFrames() method.  It's the
             * responsibility of the caller which is exchanging listeners to ensure that the old
             * listener releases any captured frames that it's holding, otherwise the next calls to
             * releaseAllBuffers() or executeUseCase() may block.
             */
            virtual void setCaptureListener (royale::collector::IFrameCaptureListener *listener) = 0;

            /**
             * Allows the bridge to get access to the sensors needed for monitoring the temperature
             * and safety of the hardware.
             *
             * Called after the Bridge, Imager and all sensors have been initialized.
             */
            virtual void setTemperatureSensor (std::shared_ptr<royale::hal::ITemperatureSensor> sensor) = 0;

            /**
             * Allows access to the temperature sensors that are read out via the pseudo data.
             *
             * Called after the Bridge, Imager and all sensors have been initialized.
             */
            virtual void setTemperatureSensor (std::shared_ptr<royale::hal::IPsdTemperatureSensor> sensor) = 0;

            /**
             * Set event listener which is supposed to be called for things like thermal excursions.
             */
            virtual void setEventListener (royale::IEventListener *listener) = 0;

            /**
             * Verification of the use case.
             *
             * This checks whether the frame collector can support the use case (i.e. whether executeUseCase
             * might succeed).
             *
             * \param   useCase   The use case definition.
             * \return  SUCCESS if verification passes, otherwise the appropriate verification status.
             */
            virtual royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) const = 0;

            /**
             * Called after sending commands to the imager that may change the buffer size, or the
             * number of raw frames in the use case.
             *
             * The lifetime of the UseCaseDefinition must be longer than the lifetime of the
             * FrameCollector, as the pointer is kept and passed to the IBufferCaptureListener in
             * each capture callback.
             *
             * The measurementBlockSizes parameter is to support imagers with mixed-mode firmware,
             * the necessary values are returned by IImager::getMeasurementBlockSizes().
             *
             * This will requeue all CapturedRawFrame instances currently held
             * by the FrameCollector, and update the parameters used in
             * bufferCallback for processing newly captured buffers.
             *
             * The FrameCollector calls the Bridge's executeUseCase (which may block waiting for
             * buffers), and releases all buffers.  Note that the buffers may be released
             * asynchronously (as the IFrameCaptureListener releases the respective frames), the
             * bridge should not assume that all buffers have already been queued before
             * IBridgeDataReceiver::executeUseCase is called.
             *
             * With this version of IFrameCollector::executeUseCase, the FrameCollector handles the
             * synchronization.  The Bridge may continue to call call bufferCallback, although this
             * is not optimal the FrameCollector will ensure that the buffer is quickly returned to
             * the Bridge, so that the Bridge's executeUseCase can finish.
             *
             * It is assumed that the IBridgeDataReceiver passed to this function is the same as the
             * IBufferCaptureReleaser that is receiving the released buffers.
             */
            virtual void executeUseCase (royale::hal::IBridgeDataReceiver &bridge,
                                         const royale::usecase::UseCaseDefinition *useCase,
                                         const royale::Vector<std::size_t> &measurementBlockSizes) = 0;

            /**
             * Update the data used for CapturedUseCase::getExposureTime, but only affecting frames
             * whose pseudodata contains a reconfig index indicating that the reconfiguration has
             * taken place.  The value will be overwritten by (and is automatically updated by) any
             * call to executeUseCase; this is for minor changes that can be made without stopping
             * the capture. The exposure times are expected in the order of the exposure groups.
             *
             * Capturing data is asynchronous from changing the Imager's settings, we only know the
             * reconfiguration counter's value before the new settings were sent to the imager.
             *
             * Some imager firmware versions send the new reconfig index in the last frame before
             * the new settings take effect, instead of waiting until the first frame with the new
             * settings.  These firmware versions are currently unsupported, and on them the
             * exposure would be reported too early.
             */
            virtual void setReportedExposureTimes (uint16_t afterFrame, const std::vector<uint32_t> &exposureTimes) = 0;

            /**
            * Wait until all pending configuration changes set with previous calls to setReportedExposureTimes()
            * have become active (i.e. a frame set with a greater reconfiguration index was encountered).
            * Note that there may still be frames with the old settings in the conveyance queue which would
            * be delivered after this function returns.
            * Will return immediately if there are no pending configuration changes.
            * Will also return if executeUseCase(), releaseAllBuffers() or the destructor is called concurrently.
            */
            virtual void syncReportedExposureTimes() = 0;

            /**
            * Check if there are any pending configuration changes set with previous calls to setReportedExposureTimes().
            *
            * \return true if there are still pending configuration changes.
            */
            virtual bool pendingReportedExposureTimes() = 0;

            /**
             * Implementation of CaptureReleaser, releases a frame of the type passed to
             * CaptureListener.  If this IFrameCollector is converting hardware superframes in to
             * individual frames, this checks whether the internal buffer can now be released.
             *
             * This should call IBufferCaptureReleaser->queueBuffer as appropriate.
             */
            virtual void releaseCapturedFrames (std::vector<royale::common::ICapturedRawFrame *> frame) override = 0;
        };
    }
}
