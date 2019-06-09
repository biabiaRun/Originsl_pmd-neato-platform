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

#include <collector/IFrameCollector.hpp>
#include <common/IPseudoDataInterpreter.hpp>
#include <royale/IEventListener.hpp>
#include <common/EventForwarder.hpp>
#include <hal/IPsdTemperatureSensor.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace royale
{
    namespace collector
    {
        /**
        * Internal structure for passing data between threads.  This is all the data needed to
        * call CaptureListener's callback.
        */
        struct CallbackData
        {
            CallbackData() = default;
            CallbackData (const CallbackData &) = delete;
            CallbackData &operator= (const CallbackData &) = delete;
            ~CallbackData() = default;

            std::vector<royale::common::ICapturedRawFrame *> frames;
            const royale::usecase::UseCaseDefinition *definition;
            royale::StreamId streamId;
            std::unique_ptr<royale::collector::CapturedUseCase> capturedCase;
        };


        struct SequenceExposureInfo
        {
            /**
             * Index of the first raw frame belonging to this exposure group
             */
            uint16_t rawFrameIdx;
            /**
             * Index of the exposure group
             */
            uint16_t exposureGroupIdx;
            /**
             * Modulation frequency for this exposure group
             */
            uint32_t modulationFrequency;
        };

        /**
         * The information that the FrameCollector needs about one of the usecase::FrameGroups.
         */
        struct CollectorFrameGroup
        {
            /**
             * The sub use case's ID
             */
            royale::StreamId streamId;
            /**
             * The expected sequence indices in the pseudodata of a successful capture.  The size
             * of this vector matches the number of frames expected.
             */
            std::vector<uint16_t> sequence;
            /**
             * The exposure information for this group
             */
            std::vector<SequenceExposureInfo> exposeq;
        };

        struct MapFramesTo
        {
            /**
             * Which CollectorFrameGroup this mapping is directed to.  The value is the index for
             * this group in m_frameGroupList and m_capturedFrames, not an ID.
             */
            std::size_t group;
            /**
             * Index in to m_capturedFrames[group] and m_frameGroupList[group].sequence.
             */
            std::size_t index;
        };

        /**
         * Buffer handling for Mixed Mode (including Interleaved Mode).  When a buffer is received,
         * the corresponding BufferAction is a mapping from RawFrames in the buffer to Frame Groups
         * and also a list of which Frame Groups should now (if complete) be sent to the listener.
         */
        struct BufferAction
        {
            /**
             * When we receive a buffer, RawFrame i of the buffer is mapped to
             * the zero or more locations in mapping[i].
             *
             * For TransmissionMode::INDIVIDUAL, mapping.size() == 1.  For superframes it will be
             * the number of RawFrames in that particular superframe; the FrameCollectorBase finds
             * the buffer action before knowing how many frames will be in the buffer.
             */
            std::vector<std::vector<MapFramesTo> > mapping;
            /**
             * Which groups are ready, often empty.
             */
            std::vector<std::size_t> ready;
        };

        using BufferActionMap = std::map<uint16_t, BufferAction>;

        /**
         * Encapsulates the logic for handling the transmission mode (superframes or individual
         * frames).
         */
        class IBufferActionCalc
        {
        public:
            ROYALE_API virtual ~IBufferActionCalc() = default;

            struct Result
            {
                BufferActionMap actions;
                uint16_t bufferWidth;
                /**
                 * The size of the images that the bridge should expect, will be multiple raw frames
                 * in superframe mode.
                 */
                uint16_t bufferHeight;
                std::size_t bufferCount;
            };

            /**
             * Called during FrameCollectorBase::executeUseCase. The caller will generate the vector
             * of CollectorFrameGroups corresponding to the UseCaseDefinition.  This function must
             * then generate the corresponding BufferActionMap.
             */
            ROYALE_API virtual Result calculateActions (
                const royale::usecase::UseCaseDefinition &useCase,
                const std::vector<CollectorFrameGroup> &frameGroupList,
                const royale::Vector<std::size_t> &blockSizes) const = 0;
        };

        /**
         * The reference implementation of IFrameCollector.
         *
         * This class owns a thread.  If this class is subclassed then most-derived subclass'
         * destructor must call releaseAllBuffersInternal(true) to stop it, otherwise there may
         * still be callbacks from that thread, and the corresponding calls back to the virtual
         * releaseCapturedFrames() method.
         */
        class FrameCollectorBase : public IFrameCollector
        {
        public:
            /**
             * Constructor.  This takes an IBufferCaptureReleaser as an argument, it is already
             * necessary for the releaser's lifespan to be longer than the useful lifespan of the
             * IFrameCollector (otherwise there are no buffers).
             *
             * \param interpreter handles the type of pseudodata that the buffers will contain
             * \param releaser the Bridge to return the buffers to
             * \param calc handles the transmission mode (superframes or individual frames)
             */
            ROYALE_API explicit FrameCollectorBase (std::unique_ptr<royale::common::IPseudoDataInterpreter> interpreter,
                                                    std::shared_ptr<royale::hal::IBufferCaptureReleaser> releaser,
                                                    std::unique_ptr<royale::collector::IBufferActionCalc> calc);
            ROYALE_API FrameCollectorBase (const FrameCollectorBase &) = delete;
            ROYALE_API FrameCollectorBase &operator= (const FrameCollectorBase &) = delete;

        protected:
            /**
             * When called with argument false, this equivalent to releaseAllBuffers(), as
             * documented in IBufferCaptureListener.
             *
             * With argument true, it additionally causes the conveyance thread to exit, which
             * should only happen as part of the destruction sequence.  If subclassing this class,
             * then this should be called by the most derived class' destructor, so that the thread
             * doesn't cause async callbacks to a partially-destroyed object.  It's okay to call it
             * multiple times during the destruction sequence.
             */
            ROYALE_API void releaseAllBuffersInternal (bool stopConveyanceThread);

        public:
            // implement IFrameCollector
            ROYALE_API ~FrameCollectorBase() override;
            ROYALE_API void setCaptureListener (royale::collector::IFrameCaptureListener *listener) override;
            ROYALE_API void setTemperatureSensor (std::shared_ptr<royale::hal::ITemperatureSensor> sensor) override;
            ROYALE_API void setTemperatureSensor (std::shared_ptr<royale::hal::IPsdTemperatureSensor> sensor) override;
            ROYALE_API void setEventListener (royale::IEventListener *listener) override;
            ROYALE_API royale::usecase::VerificationStatus verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) const override;
            ROYALE_API void executeUseCase (royale::hal::IBridgeDataReceiver &bridge,
                                            const royale::usecase::UseCaseDefinition *useCase,
                                            const royale::Vector<std::size_t> &blockSizes) override;
            ROYALE_API void setReportedExposureTimes (uint16_t afterFrame, const std::vector<uint32_t> &exposureTimes) override;
            ROYALE_API void releaseAllBuffers() override;
            ROYALE_API void syncReportedExposureTimes() override;
            ROYALE_API bool pendingReportedExposureTimes() override;
            ROYALE_API void releaseCapturedFrames (std::vector<royale::common::ICapturedRawFrame *> frame) override;
            ROYALE_API void bufferCallback (royale::hal::ICapturedBuffer *buffer) override;

            /**
            * Reset raw frame drop statistics, optionally generating an event (of type RawFrameStatsEvent).
            * Mainly intended for unit testing and therefore not part of the IFrameCollector interface.
            *
            * \param generateEvent true if an event should be generated
            */
            ROYALE_API void flushRawFrameStatistics (bool generateEvent);

        private:
            // Helper functions

            /** Notify client that something has happened with the stream. */
            void reportErrorToClient (const royale::String &message);

            /**
            * If m_temperatureReading was last updated longer than threshold ago, makes an
            * immediate reading and updates both the reading and the timestamp.
            *
            * If m_temperatureSensor is null, this will return without error, but without updating
            * the m_temperatureReading.
            *
            * \param threshold maximum age of the old reading
            */
            void updateTemperature (std::chrono::duration<int, std::milli> threshold);

            /**
             * This is an internal part of the implementation of bufferCallback.  After all frames
             * from the buffer have been added to m_capturedFrames, this will check the groups that
             * are expected to be ready to be sent to the IFrameCaptureListener.
             *
             * Complete groups are added to the m_conveyanceQueue, incomplete groups have the any
             * captured frames released.  In either case, the respective vectors in m_capturedFrames
             * are emptied, they remain the same size, but all the entries are nullptr.
             *
             * Called by bufferCallback() with m_bufferLock being held.
             */
            void processReadyGroups (const decltype (BufferAction::ready) &ready);

            /**
             * Note down incoming frame numbers for statistics, for detecting frames dropped by the bridge.
             * To be called from bufferCallback(), or one of the functions called from bufferCallback().
             *
             * \param frameNumber Frame number of the first raw frame in the current buffer
             * \param count Number of raw frame in the current buffer
             */
            void updateFrameNumber (uint16_t frameNumber, std::size_t count);

            /**
            * Update frame drop statistics.
            *
            * To be called from collectFrames. Can be called multiple times.
            *
            * \param nDiscarded  Number of frames discarded by frame collector
            * \param nAccepted   Number of frames forwarded to the conveyance queue
            *
            */
            void updateStats (size_t nDiscarded, size_t nAccepted);

            /**
            * Internal version of flushRawFrameStatistics(bool).
            * Assumes the caller owns m_bufferLock.
            */
            void flushRawFrameStatisticsInternal (bool generateEvent);

            /**
            * Check frame for reconfiguration according to m_pendingExposureTimes
            * and update m_exposureTimes accordingly.
            * Will also wake up threads waiting on m_reconfigCV if needed.
            *
            * Caller should hold m_bufferLock.
            */
            void checkForReconfig (uint16_t reconfigIndex);

            /**
            * Clear m_pendingExposureTimes and wake up anyone blocking on m_reconfigCV.
            *
            * Caller should hold m_bufferLock.
            */
            void clearPendingExposureTimes();

            /**
            * Checks the eye safety monitor status (if it's available)
            * and generates an event if the monitor was triggered.
            */
            void updateEyeSafetyErrorAndState (const royale::common::ICapturedRawFrame &frame);

        private:
            /**
            * The loop that is the m_conveyanceThread.
            */
            void conveyanceFunction();

            /** Internal implementation of releaseCapturedFrames */
            void releaseCapturedFrame (royale::common::ICapturedRawFrame *frame);

            /**
             * Takes a vector which may contain nullptrs, on return the vector is the same size but
             * contains only nullptrs.  All non-nullptrs are passed to releaseCapturedFrame.
             *
             * This is a cleanup function for the vectors in m_capturedFrames, and also used in the
             * conveyance thread's error handling.  It assumes the caller holds any locks required
             * for the vector that's passed as an argument.
             */
            void releaseSparseCollection (std::vector<royale::common::ICapturedRawFrame *> &frame);

            /**
             * The part of executeUseCase that needs the m_bufferLock. Releases all buffers that are
             * in the frame collector, and then sets up all of the member data such as
             * m_pixelsPerFrame that should only be changed with the m_bufferLock held.
             *
             * Synchronization: the caller should hold m_executeUseCaseLock, but not m_bufferLock.
             */
            void executeUseCaseInternal (const royale::usecase::UseCaseDefinition *useCase, const std::vector<CollectorFrameGroup> &groups, const BufferActionMap &actions);

        private:
            // data members

            /** Where to pass the captured frames on to */
            royale::collector::IFrameCaptureListener *m_captureListener;

            /** Where to return the buffers to */
            std::shared_ptr<royale::hal::IBufferCaptureReleaser> m_bufferReleaser;

            std::vector<CollectorFrameGroup> m_frameGroupList;
            BufferActionMap m_bufferActionMap;
            std::unique_ptr<royale::collector::IBufferActionCalc> m_bufferActionCalc;

        protected:
            /**
             * The part of the UseCaseDefinition needed to validate frames that are passed to
             * collectFrames, and use for determining when to call the CaptureListener.
             */
            std::size_t m_useCaseFrameCount;

            /**
             * Synchronization between the executeUseCase and bufferCallback.  This will be
             * locked by executeUseCase (in all cases), and bufferCallback (if it processes the
             * buffer).  However, if bufferCallback is called when it's already locked,
             * bufferCallback will immediately release the buffer that it was called with.
             *
             * Lock ordering: must be acquired before m_bufferLock.
             */
            std::mutex m_executeUseCaseLock;

        private:
            /**
             * Number of pixels in each row of the data, also the number of pixels in the pseudodata
             * row before the image data.
             */
            std::size_t m_imageWidth;

            /** Number of pixels in a whole frame. */
            std::size_t m_pixelsPerFrame;

            // Temperature

            /**
            * The source of the measurement passed to the CaptureListener via a CapturedUseCase.
            *
            * Will be nullptr until setTemperatureSensor() is called.
            */
            std::shared_ptr<royale::hal::ITemperatureSensor> m_temperatureSensor;

            /**
            * The source of the temperature measurement via pseudo data from the imager.
            *
            * Will be nullptr until setTemperatureSensor() is called.
            */
            std::shared_ptr<royale::hal::IPsdTemperatureSensor> m_psdTemperatureSensor;

            /**
            * Protection for reading the temperature, and ensuring that there aren't simultaneous
            * I/O requests from the FrameCollector to the temperature sensor.
            *
            * Lock ordering: this should be the last lock acquired
            */
            std::mutex m_temperatureLock;

            /**
            * Most recent measurement from the m_temperatureSensor.
            *
            * Only updated with the m_temperatureLock held, but accessed on any thread.
            */
            std::atomic<float> m_temperatureReading;

            /**
            * When was m_temperatureReading last updated.
            *
            * Only updated with the m_temperatureLock held, but accessed on any thread.
            */
            std::chrono::steady_clock::time_point m_temperatureTimestamp;


            // Conveyance

            /**
            * Thread that calls CaptureListener::captureCallback (the processing runs in this thread).
            */
            std::thread m_conveyanceThread;

            /**
             * Protection for changing m_capturedFrames or m_conveyanceQueue, also used to protect
             * things that should be synchronized with that (m_useCaseDefinition,
             * m_useCaseFrameCount, m_frameNumberOfSequenceBase, m_psdTemperatureSensor).
             */
            std::mutex m_bufferLock;

            /**
            * Triggered by collectFrames when frames are ready for the conveyance thread.
            * Use in conjunction with m_bufferLock.
            */
            std::condition_variable m_conveyanceCV;

            /**
            * Protection for the contract of IFrameCaptureListener, to ensure that the listener's
            * captureCallback and releaseAllFrames aren't called simultaneously.  A call to
            * releaseAllBuffers will block until it can take this lock, while the conveyance thread
            * will simply release buffers if releaseAllBuffers is holding the lock.
            *
            * This is also used to synchronise changes to m_captureListener.
            */
            std::recursive_mutex m_callbackLock;

            /**
            * Control variable, set to true to signal the m_conveyanceThread to finish.
            *
            * Must only be set in the destructor.  This implementation does not support restarting
            * the conveyance thread after stopping it.
            *
            * Must only be modified with the m_bufferLock held, and the m_conveyanceThread must
            * have at least one point in its loop where this is tested with the m_bufferLock held.
            */
            bool m_stopConveyance;

            /**
            * Control variable, set to true to signal the m_conveyanceThread to return ownership of
            * the buffers to the IBufferCaptureReleaser; it is set to false by m_conveyanceThread
            * once all waiting frames have been released.
            *
            * Must only be modified with the m_bufferLock held, and the m_conveyanceThread must
            * have at least one point in its loop where this is tested with the m_bufferLock held.
            */
            bool m_releaseAllBuffers;

            /**
            * Complete sets of data ready for calling the CaptureListener.  These will be handled
            * on the conveyance thread.
            *
            * This should only be accessed with the m_bufferLock held.
            */
            std::queue<CallbackData *> m_conveyanceQueue;


            // Captured frames

            /**
             * Captured frames, not including those in the m_conveyanceQueue.  The size of this
             * double-array is set in executeUseCaseInternal().
             */
            std::vector<std::vector<royale::common::ICapturedRawFrame *> > m_capturedFrames;
            bool m_sequenceBaseKnown {false};
            /** Frame number the start of the current imager sequence. Value is not meaningful when m_sequenceBaseKnown is false. */
            uint16_t m_frameNumberOfSequenceBase {0};

            /**
             * The expected value of local variable "sequence" in the next call to bufferCallback,
             * assuming that no buffers are dropped in the IBridgeDataReceiver.
             *
             * The m_bufferActionMap's actions are triggered by the buffers received in
             * bufferCallback.  If some buffers of the current sequence are dropped, this variable
             * allows any frames that those actions would pass to the processing to be released, and
             * therefore allows the related buffers to be released.
             *
             * Value is not meaningful when m_sequenceBaseKnown is false.
             */
            uint16_t m_sequenceNumberNextExpected {0};

            // Use case etc

            std::unique_ptr<royale::common::IPseudoDataInterpreter> m_pseudoDataInterpreter;

            /**
            * Data about the sequences that should be sent to the CaptureListener.
            */
            const royale::usecase::UseCaseDefinition *m_useCaseDefinition;

            /**
            * Reported to the listener in CapturedUseCase.
            */
            std::vector <uint32_t> m_exposureTimes;


            // Reconfiguration

            /**
            * Condition variable for imager reconfiguration.
            * Gets signalled when a frame set with updated reconfigIndex (i.e. new exposure times)
            * is encountered in the frameCallback context.
            * Should be used if blocking for pending reconfiguration is needed.
            *
            * Use in conjunction with m_bufferLock.
            */
            std::condition_variable m_reconfigCV;

            /**
            * From frame m_pendingExposureTimes.first onwards, m_exposureTimes will be changed m_pendingExposureTimes.second.
            *
            * Only updated while holding m_bufferLock. Values are added in
            * setReportedExposureTimes, and processed at the appropriate frame in the thread that
            * calls bufferCallback.  Calling executeUseCase will clear this as well as setting
            * m_exposureTimes.
            */
            std::queue<std::pair<uint16_t, std::vector<uint32_t> > > m_pendingExposureTimes;


            // Statistics
            /*
             * These are only accessed with the m_bufferLock held.  Normally they're only accessed
             * in the frameCallback context (via updateFrameNumber()/updateStats() called from the
             * collectFrame() implementations), with the exception of m_nFramesPerEvent which gets
             * new values in executeUseCase().  In all of these situations, the surrounding code
             * already needs to hold the m_bufferLock.
             *
             * The method flushRawFrameStatistics() also takes the m_bufferLock.
             */
            size_t   m_nFrameDropsBridge;    ///< Number of frames dropped by the bridge
            size_t   m_nFrameDrops;          ///< Number of frames discarded by the frame collector
            size_t   m_nFrameAccepts;        ///< Number of frames forwarded to conveyance queue
            uint16_t m_lastFrameNumberSeen;  ///< used to detect gaps
            size_t   m_nFramesPerEvent;      ///< Number of frames to be counted before generating an event

            // Events

            /** Event source */
            royale::EventForwarder m_eventForwarder;
        };
    }
}
