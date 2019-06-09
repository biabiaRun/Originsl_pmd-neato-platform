/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <collector/FrameCollectorBase.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/events/EventCaptureStream.hpp>
#include <common/events/EventEyeSafety.hpp>
#include <common/events/EventRawFrameStats.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;
using namespace royale::hal;

namespace
{
    /**
     * If the last temperature reading is older than this, read the temperature soon.  But the old
     * value is still fresh enough to use for the next captureCallback.
     */
    std::chrono::duration<int, std::milli> OPPORTUNISTIC_TEMPERATURE_UPDATE{ 750 };

    /**
     * If the last temperature reading is older than this threshold, read the temperature
     * immediately, delaying the captureCallback.
     */
    std::chrono::duration<int, std::milli> IMMEDIATE_TEMPERATURE_UPDATE{ 1000 };
}

namespace
{
    class BufferHolder;

    /**
     * Only for temporaries for calling IPseudoDataInterpreter, does not hold ownership
     * of the buffer or BufferHolder.
     */
    class PseudoDataQuery : public ICapturedRawFrame
    {
    public:
        PseudoDataQuery (ICapturedBuffer *buffer, std::size_t pseudoOffset) :
            m_buffer {buffer},
            m_pseudoOffset {pseudoOffset}
        {
        }

        uint16_t *getImageData() override
        {
            throw LogicError ("This class is only for querying the pseudo data");
        }

        const uint16_t *getPseudoData() const override
        {
            return &m_buffer->getPixelData() [m_pseudoOffset];
        }

    private:
        ICapturedBuffer *m_buffer;
        std::size_t m_pseudoOffset;
    };

    /**
     * The simplest conversion from Buffers to RawFrames - just
     * put a class around them.
     */
    class WrappedRawFrame : public ICapturedRawFrame
    {
    public:
        /**
         * Constructor.  Pixels include both the image data and the pseudo data.
         *
         * \param holder owner of the buffer, and bookkeeping for the refcount
         * \param buffer where the data is stored, must have a equal or longer lifespan than this class
         * \param imageOffset where in the buffer's pixels the UCD's image data starts (offset in pixels, not bytes)
         * \param pseudoOffset where in the buffer's pixels pseudo data starts (offset in pixels, not bytes)
         */
        WrappedRawFrame (BufferHolder *holder, ICapturedBuffer *buffer, std::size_t imageOffset, std::size_t pseudoOffset) :
            m_holder {holder},
            m_buffer {buffer},
            m_imageOffset {imageOffset},
            m_pseudoOffset {pseudoOffset}
        {
        }

        WrappedRawFrame (const WrappedRawFrame &) = delete;
        WrappedRawFrame &operator= (const WrappedRawFrame &) = delete;

        BufferHolder *getHolder()
        {
            return m_holder;
        }

        uint64_t getTimeMicroseconds() const
        {
            return m_buffer->getTimeMicroseconds();
        }

        uint16_t *getImageData() override
        {
            return &m_buffer->getPixelData() [m_imageOffset];
        }

        const uint16_t *getPseudoData() const override
        {
            return &m_buffer->getPixelData() [m_pseudoOffset];
        }

        ~WrappedRawFrame()
        {
        }

    private:
        BufferHolder *m_holder;
        ICapturedBuffer *m_buffer;
        std::size_t m_imageOffset;
        std::size_t m_pseudoOffset;
        friend class BufferHolder;
    };

    /**
     * Each ICapturedBuffer can map to many ICapturedRawFrames.  This the the tool that handles the
     * ref counts, and creates WrappedRawFrames on demand.
     *
     * The refcount itself is threadsafe, this class handles the thread safety.  However, the caller
     * must ensure that the return value of releaseWrapper is always checked, and that the
     * BufferHolder isn't deleted in one thread while another thread is still calling getWrapper.
     */
    class BufferHolder
    {
    public:
        BufferHolder (ICapturedBuffer *buffer, IBufferCaptureReleaser *releaser, std::size_t imageOffset, std::size_t pseudoOffset, std::size_t pixelsPerFrame) :
            m_buffer {buffer},
            m_releaser {releaser},
            m_wrapperCount {0},
            m_imageOffset {imageOffset},
            m_pseudoOffset {pseudoOffset},
            m_pixelsPerFrame {pixelsPerFrame}
        {
        }

        BufferHolder (const BufferHolder &) = delete;
        BufferHolder &operator= (const BufferHolder &) = delete;

        void checkMaxFrameCount (std::size_t frameCount)
        {
            if (m_buffer->getPixelCount() < m_pixelsPerFrame * frameCount)
            {
                throw LogicError ("out of bounds trying to create more frames than there are pixels");
            }
        }

        /**
         * Returns one of the frames within the buffer, incrementing the refcount.
         */
        WrappedRawFrame *getWrapper (std::size_t frame)
        {
            auto basePixel = frame * m_pixelsPerFrame;
            m_wrapperCount++;
            return new WrappedRawFrame (this, m_buffer, basePixel + m_imageOffset, basePixel + m_pseudoOffset);
        }

        /**
         * Returns an object allowing queries on the pseudodata of a frame.  This doesn't have a
         * refcount, so is faster but becomes invalid when there are no owning references to the
         * BufferHolder.
         */
        PseudoDataQuery getPseudoData (std::size_t frame)
        {
            auto basePixel = frame * m_pixelsPerFrame;
            return PseudoDataQuery (m_buffer, basePixel + m_pseudoOffset);
        }

        /**
         * Decrements the refcount and frees the WrappedRawFrame.
         *
         * @return true if the last reference was removed; the caller should then free the BufferHolder
         */
        bool releaseWrapper (WrappedRawFrame *frame)
        {
            if (m_buffer != frame->m_buffer)
            {
                throw LogicError ("buffer owners do not match");
            }
            delete frame;

            auto refCount = --m_wrapperCount;
            if (refCount < 0)
            {
                throw LogicError ("negative refcount");
            }
            return refCount == 0;
        }

        ~BufferHolder ()
        {
            if (m_wrapperCount)
            {
                LOG (ERROR) << "LogicError: deleting a BufferHolder that is still ref'd by WrappedRawFrames";
            }
            try
            {
                m_releaser->queueBuffer (m_buffer);
            }
            catch (const Exception &e)
            {
                LOG (ERROR) << "FrameCollector caught exception while requeueing buffer: " << e.what();
            }
        }

    private:
        ICapturedBuffer *m_buffer;
        IBufferCaptureReleaser *m_releaser;
        std::atomic<int> m_wrapperCount;
        std::size_t m_imageOffset;
        std::size_t m_pseudoOffset;
        std::size_t m_pixelsPerFrame;
    };
}

FrameCollectorBase::FrameCollectorBase (std::unique_ptr<IPseudoDataInterpreter> interpreter,
                                        std::shared_ptr<IBufferCaptureReleaser> releaser,
                                        std::unique_ptr<IBufferActionCalc> calc) :
    m_captureListener{ nullptr },
    m_bufferReleaser{ std::move (releaser) },
    m_bufferActionCalc{ std::move (calc) },
    m_useCaseFrameCount{ 0 },
    m_temperatureSensor{ nullptr },
    m_temperatureLock(),
    m_temperatureReading{ 0.0f },
    m_temperatureTimestamp(),
    m_conveyanceThread(),
    m_bufferLock(),
    m_conveyanceCV(),
    m_stopConveyance{ false },
    m_releaseAllBuffers{ false },
    m_conveyanceQueue(),
    m_capturedFrames(),
    m_pseudoDataInterpreter{ std::move (interpreter) },
    m_useCaseDefinition{ nullptr },
    m_exposureTimes(),
    m_reconfigCV(),
    m_pendingExposureTimes(),
    m_nFrameDropsBridge (0),
    m_nFrameDrops (0),
    m_nFrameAccepts (0),
    m_lastFrameNumberSeen (0),
    m_nFramesPerEvent (1),
    m_eventForwarder()
{
    // Setting m_useCaseFrameCount to zero means there will be a warning if collectFrames is called
    // without calling executeUseCase first.  Initialising m_frameNumberOfSequenceBase isn't necessary.
    m_conveyanceThread = std::thread (&FrameCollectorBase::conveyanceFunction, this);
}

FrameCollectorBase::~FrameCollectorBase()
{
    releaseAllBuffersInternal (true);
    // m_stopConveyance is now set and the thread is stopped.  A sanity check in bufferCallback()
    // will ensure that no new buffers are added to the queue, even if the Bridge calls us again.

    // The class that owns the Bridge and FrameCollector is required to set the Bridge's listener
    // pointer to nullptr before destroying the FrameCollector.  But now that the Bridge is a
    // shared_ptr, this class may have the last reference to the Bridge; clear it so that (if
    // clearing it triggers the Bridge's destructor), any calls from the Bridge happen before this
    // class is destroyed.
    m_bufferReleaser.reset();

    // Unblock any caller of syncReportedExposureTimes()
    std::lock_guard<std::mutex> lock (m_bufferLock);
    clearPendingExposureTimes();
}

void FrameCollectorBase::setCaptureListener (IFrameCaptureListener *listener)
{
    std::lock_guard<std::recursive_mutex> callbackLock (m_callbackLock);
    m_captureListener = listener;
}

void FrameCollectorBase::setTemperatureSensor (std::shared_ptr<ITemperatureSensor> sensor)
{
    std::lock_guard<std::mutex> lock (m_temperatureLock);
    m_temperatureSensor = sensor;
}

void FrameCollectorBase::setTemperatureSensor (std::shared_ptr<IPsdTemperatureSensor> sensor)
{
    std::lock_guard<std::mutex> lock (m_bufferLock);
    m_psdTemperatureSensor = sensor;
}

void FrameCollectorBase::setEventListener (royale::IEventListener *listener)
{
    m_eventForwarder.setEventListener (listener);
}

void FrameCollectorBase::setReportedExposureTimes (uint16_t afterFrame, const std::vector<uint32_t> &exposureTimes)
{
    std::lock_guard<std::mutex> lock (m_bufferLock);
    if (exposureTimes.size() != m_useCaseDefinition->getExposureGroups().size())
    {
        throw LogicError ("Updated exposure times don't match the current UCD");
    }

    if (!m_pendingExposureTimes.empty())
    {
        auto lastPendingFrame = m_pendingExposureTimes.back().first;
        if (m_pseudoDataInterpreter->isGreaterFrame (afterFrame, lastPendingFrame))
        {
            throw LogicError ("Pending data is already set for a later frame");
        }
    }

    if (!m_pseudoDataInterpreter->supportsExposureFromPseudoData())
    {
        m_pendingExposureTimes.emplace (afterFrame, exposureTimes);
    }
}

void FrameCollectorBase::releaseAllBuffersInternal (bool stopConveyanceThread)
{
    // Set m_releaseAllBuffers and wait for the conveyance queue to unset it, even if there are
    // no buffers in m_conveyanceCV. This ensures that we wait if the conveyance thread is in
    // the callback.

    // scope for the conveyance lock
    {
        std::unique_lock<std::mutex> lock (m_bufferLock);
        for (auto &group : m_capturedFrames)
        {
            releaseSparseCollection (group);
        }
        m_releaseAllBuffers = true;
        if (stopConveyanceThread)
        {
            m_stopConveyance = true;
        }
        m_conveyanceCV.notify_all();
        m_conveyanceCV.wait (lock, [this] { return m_conveyanceQueue.empty() && !m_releaseAllBuffers; });
    }
    if (stopConveyanceThread)
    {
        m_conveyanceThread.join();
    }

    // also throw away any pending reconfigurations
    clearPendingExposureTimes();

    // The code above is sufficient for our current synchronous Processing implementation, where all
    // calls to releaseCapturedFrame() are made before the callback returns.  This final call is to
    // provide support for an asynchronous Processing implementation.
    //
    // Although the callback lock needs to be held for longer, defer taking it until after the
    // main m_bufferLock.  If the conveyance thread is busy, then it will be holding the
    // callbackLock for most of the time, with very short time windows for this thread to take it.
    //
    // The callback lock is unlikely to be needed because of emptying the conveyance queue, but
    // an imager using superframes might be able to create a full set of frames at exactly the right
    // timing to trigger another capture.
    std::lock_guard<std::recursive_mutex> callbackLock (m_callbackLock);
    if (m_captureListener)
    {
        m_captureListener->releaseAllFrames ();
    }
}

void FrameCollectorBase::releaseAllBuffers()
{
    releaseAllBuffersInternal (false);
}

royale::usecase::VerificationStatus FrameCollectorBase::verifyUseCase (const royale::usecase::UseCaseDefinition &useCase) const
{
    if (useCase.getStreamIds().empty())
    {
        return VerificationStatus::STREAM_COUNT;
    }

    // Check image size against PDI requirements
    auto minWidth = m_pseudoDataInterpreter->getRequiredImageWidth();
    uint16_t nCols, nRows;
    useCase.getImage (nCols, nRows);

    if (nCols < minWidth)
    {
        return VerificationStatus::REGION;
    }

    return VerificationStatus::SUCCESS;
}

void FrameCollectorBase::executeUseCase (royale::hal::IBridgeDataReceiver &bridge,
        const royale::usecase::UseCaseDefinition *useCase,
        const royale::Vector<std::size_t> &blockSizes)
{
    std::lock_guard<std::mutex> eucLock (m_executeUseCaseLock);

    // Verify first.
    if (verifyUseCase (*useCase) != VerificationStatus::SUCCESS)
    {
        throw LogicError ("Use case not valid for frame collector");
    }

    // Constructing the frame group list is independent of how the frames will be transported

    const auto streamIds = useCase->getStreamIds();
    const auto &rfs = useCase->getRawFrameSets();

    std::vector<CollectorFrameGroup> frameGroupList;
    for (const auto &streamId : streamIds)
    {
        const auto groupCount = useCase->getFrameGroupCount (streamId);
        for (std::size_t groupIdx = 0u; groupIdx < groupCount ; groupIdx++)
        {
            CollectorFrameGroup cg;
            cg.streamId = streamId;

            for (const auto rawFrameSetId : useCase->getRawFrameSetIndices (streamId, groupIdx))
            {
                const auto &seqIdxs = useCase->getSequenceIndicesForRawFrameSet (rawFrameSetId);

                SequenceExposureInfo se;
                se.rawFrameIdx = static_cast<uint16_t> (cg.sequence.size());
                se.exposureGroupIdx = rfs[rawFrameSetId].exposureGroupIdx;
                se.modulationFrequency = rfs[rawFrameSetId].modulationFrequency;
                cg.exposeq.push_back (se);

                for (const auto i : seqIdxs)
                {
                    cg.sequence.push_back (i);
                }
            }

            frameGroupList.push_back (std::move (cg));
        }
    }

    auto calcs = m_bufferActionCalc->calculateActions (*useCase, frameGroupList, blockSizes);

    executeUseCaseInternal (useCase, frameGroupList, calcs.actions);

    auto buffersAllocated = bridge.executeUseCase (calcs.bufferWidth, calcs.bufferHeight, calcs.bufferCount);
    if (buffersAllocated < calcs.bufferCount)
    {
        LOG (WARN) << "The bridge can't allocate sufficient buffers to fully support this use case";
    }
}

void FrameCollectorBase::executeUseCaseInternal (const UseCaseDefinition *useCase, const std::vector<CollectorFrameGroup> &groups, const BufferActionMap &actions)
{
    // This is called with the eucLock held, so once releaseAllBuffers() returns there are no
    // buffers in the frame collector, and the m_bufferLock can be locked immediately.
    releaseAllBuffers();
    std::lock_guard<std::mutex> lock (m_bufferLock);

    m_frameGroupList = groups;
    m_bufferActionMap = actions;
    m_useCaseFrameCount = useCase->getRawFrameCount();
    m_useCaseDefinition = useCase;
    m_nFramesPerEvent = m_useCaseFrameCount * useCase->getTargetRate(); // roughly one event per second

    m_capturedFrames.resize (groups.size());
    for (std::size_t i = 0; i < groups.size(); i++)
    {
        m_capturedFrames[i] = std::vector<ICapturedRawFrame *> (groups[i].sequence.size(), nullptr);
    }

    // Get the width and height of each frame's image, in pixels
    uint16_t imageWidth;
    uint16_t imageHeight;
    useCase->getImage (imageWidth, imageHeight);
    // add pseudodata
    imageHeight++;
    m_imageWidth = imageWidth;
    m_pixelsPerFrame = imageWidth * imageHeight;

    m_exposureTimes = useCase->getExposureTimes().toStdVector();
    clearPendingExposureTimes();
}

void FrameCollectorBase::flushRawFrameStatistics (bool generateEvent)
{
    std::unique_lock<std::mutex> lock (m_bufferLock);
    flushRawFrameStatisticsInternal (generateEvent);
}

void FrameCollectorBase::syncReportedExposureTimes()
{
    std::unique_lock<std::mutex> lock (m_bufferLock);
    m_reconfigCV.wait (lock, [this] { return m_pendingExposureTimes.empty(); });
}

bool FrameCollectorBase::pendingReportedExposureTimes()
{
    std::unique_lock<std::mutex> lock (m_bufferLock);
    return !m_pendingExposureTimes.empty();
}

void FrameCollectorBase::reportErrorToClient (const royale::String &message)
{
    m_eventForwarder.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING, message);
}

void FrameCollectorBase::updateTemperature (std::chrono::duration<int, std::milli> threshold)
{
    auto timestamp = std::chrono::steady_clock::now();
    if (timestamp - m_temperatureTimestamp > threshold)
    {
        try
        {
            // Take the lock, and then check that the reading hasn't been updated by another
            // thread.
            std::lock_guard<std::mutex> lock (m_temperatureLock);
            if (m_temperatureSensor && (timestamp - m_temperatureTimestamp > threshold))
            {
                m_temperatureReading = m_temperatureSensor->getTemperature();
                m_temperatureTimestamp = timestamp;
            }
        }
        catch (Exception &e)
        {
            // Getting here means that we can receive data, but don't have the ability to send I2C
            // commands to the module.  Or that the I2C address is incorrectly configured.
            LOG (ERROR) << "Error while trying to read the temperature" << e.what();
            reportErrorToClient ("Error while trying to read the temperature");
        }
    }
}

void FrameCollectorBase::updateFrameNumber (uint16_t frameNumber, std::size_t count)
{
    auto dist = m_pseudoDataInterpreter->frameNumberFwdDistance (frameNumber, m_lastFrameNumberSeen);
    m_lastFrameNumberSeen = m_pseudoDataInterpreter->getFollowingFrameNumber (frameNumber, static_cast<uint16_t> (count - 1));
    if (dist > 1)
    {
        m_nFrameDropsBridge += dist - 1;
    }
}

void FrameCollectorBase::updateStats (size_t nDiscarded, size_t nAccepted)
{
    m_nFrameDrops   += nDiscarded;
    m_nFrameAccepts += nAccepted;
    if (m_nFrameAccepts + m_nFrameDrops + m_nFrameDropsBridge >= m_nFramesPerEvent)
    {
        flushRawFrameStatisticsInternal (true);
    }
}

void FrameCollectorBase::flushRawFrameStatisticsInternal (bool generateEvent)
{
    m_eventForwarder.event<event::EventRawFrameStats> (narrow_cast<uint16_t> (m_nFrameAccepts + m_nFrameDrops + m_nFrameDropsBridge),
            narrow_cast<uint16_t> (m_nFrameDropsBridge),
            narrow_cast<uint16_t> (m_nFrameDrops));
    m_nFrameDrops = 0;
    m_nFrameAccepts = 0;
    m_nFrameDropsBridge = 0;
}

void FrameCollectorBase::checkForReconfig (const uint16_t capturedReconfigIndex)
{
    // caller holds m_bufferLock.

    if (m_pendingExposureTimes.empty())
    {
        return;    // no pending reconfigurations.
    }

    while ( (!m_pendingExposureTimes.empty()) &&
            m_pseudoDataInterpreter->isGreaterFrame (m_pendingExposureTimes.front().first, capturedReconfigIndex))
    {
        // We have a reconfiguration. Activate updated exposure times and unblock syncReportedExposureTimes().
        m_exposureTimes = m_pendingExposureTimes.front().second;
        m_pendingExposureTimes.pop();
        m_reconfigCV.notify_all();
    }
}

void FrameCollectorBase::clearPendingExposureTimes()
{
    // caller holds m_bufferLock.
    while (!m_pendingExposureTimes.empty())
    {
        m_pendingExposureTimes.pop();
    }
    m_reconfigCV.notify_all();
}

void FrameCollectorBase::conveyanceFunction()
{
    // Each loop will remove exactly one CallbackData from the queue and either process or release
    // it.  The destructor will set m_stopConveyance to signal this thread to finish, and the local
    // runConveyance variable will be cleared once all queued buffers have been released.
    bool runConveyance = true;
    while (runConveyance)
    {
        if (m_temperatureSensor && !m_stopConveyance && !m_releaseAllBuffers)
        {
            updateTemperature (OPPORTUNISTIC_TEMPERATURE_UPDATE);
        }

        CallbackData *callback = nullptr;

        // scope for the lock
        {
            std::unique_lock<std::mutex> lock (m_bufferLock);
            m_conveyanceCV.wait (lock, [this] { return (m_releaseAllBuffers || m_stopConveyance || !m_conveyanceQueue.empty()); });
            if (m_conveyanceQueue.empty())
            {
                if (m_releaseAllBuffers)
                {
                    m_releaseAllBuffers = false;
                    m_conveyanceCV.notify_all();
                }
                if (m_stopConveyance)
                {
                    runConveyance = false;
                }
            }
            else
            {
                callback = m_conveyanceQueue.front();
                m_conveyanceQueue.pop();
            }
        }
        if (callback)
        {
            bool ownershipPassedToListener = false;
            if (m_releaseAllBuffers || m_stopConveyance)
            {
                // this iteration is just to release the captured buffers
            }
            else
            {
                std::unique_lock<std::recursive_mutex> callbackLock (m_callbackLock, std::try_to_lock);
                if (!callbackLock.owns_lock())
                {
                    LOG (DEBUG) << "Successful frame collection, but releasing all buffers.  frames: " << callback->frames.size();
                }
                else if (!m_captureListener)
                {
                    LOG (DEBUG) << "Successful frame collection, but no CaptureListener.  frames: " << callback->frames.size();
                }
                else
                {
                    try
                    {
                        ownershipPassedToListener = true;
                        m_captureListener->captureCallback (callback->frames, *callback->definition, callback->streamId, std::move (callback->capturedCase));
                    }
                    catch (...)
                    {
                        LOG (DEBUG) << "Exception during captureCallback";
                        reportErrorToClient ("Exception during captureCallback");
                    }
                }
            }
            if (!ownershipPassedToListener)
            {
                releaseCapturedFrames (callback->frames);
            }
            delete callback;
        }
    }
}

void FrameCollectorBase::bufferCallback (royale::hal::ICapturedBuffer *buffer)
{
    std::unique_lock<std::mutex> eucLock (m_executeUseCaseLock, std::try_to_lock);
    if (!eucLock.owns_lock())
    {
        LOG (WARN) << "FRAME DROP in bufferCallback: eucLock";
        m_bufferReleaser->queueBuffer (buffer);
        return;
    }

    // The holders can take a bare pointer to m_bufferReleaser. They shouldn't outlive the
    // FrameCollectorBase itself, as they should be passed back to the FrameCollector.
    // \todo ROYAL-2073: Implementing ROYAL-2073 would invalidate this assumption.
    auto holder = makeUnique<BufferHolder> (buffer, m_bufferReleaser.get(), m_imageWidth, 0, m_pixelsPerFrame);
    buffer = nullptr;

    auto firstFrame = holder->getPseudoData (0);
    const auto frameNumber = m_pseudoDataInterpreter->getFrameNumber (firstFrame);
    const auto sequence = m_pseudoDataInterpreter->getSequenceIndex (firstFrame);
    const auto capturedReconfigIndex = m_pseudoDataInterpreter->getReconfigIndex (firstFrame);

    std::lock_guard<std::mutex> lock (m_bufferLock);

    if (m_stopConveyance)
    {
        // The destructor has already been called, but the Bridge is pushing data to us.  We can't
        // handle that (if the Bridge had called later, the FrameCollectorSuper would already
        // be destroyed).
        throw LogicError ("FrameCollector received frames during / after destruction");
    }

    // Check for either data corruption or logic corruption. This is probably data corruption of the
    // individual buffer, so don't do any other processing based on it.
    if (0 == m_bufferActionMap.count (sequence))
    {
        LOG (WARN) << "No action for sequence " << uint32_t {sequence} << " in bufferCallback";
        // this doesn't update the drop stats, instead the number of drops will be updated with
        // the number of lost frames when a known frame number is received
        return;
    }

    bool resetSequenceBase = false;
    if (!m_sequenceBaseKnown)
    {
        resetSequenceBase = true;
    }
    else
    {
        // The (frameNumber != expectedFrame) block here looks like error handling, but it and
        // resetSequenceBase are routinely used for the first frame of each sequence.
        //
        // Assume we're running an 11-frame use case, and m_frameNumberOfSequenceBase==3833.
        //
        // We receive frame 3843, with sequence number 10 (11th of a zero-based count).  That
        // finishes the sequence, but doesn't change m_frameNumberOfSequenceBase.
        //
        // The first frame of the next sequence arrives, frame 3844 with sequence number zero.
        // The expectedFrame will be calculated as if this frame was part of 3833's sequence:
        //     expectedFrame == 3833
        //     expectedStartOfNextSequence == 3844
        //     isGreaterFrame (3844, 3844) == false
        // Therefore resetSequenceBase is set.
        //
        // Even if the Bridge had dropped a few frames, isGreaterFrame will still return false for
        // the first frame that we receive, and the following frameNumberFwdDistance calculation
        // will still set m_frameNumberOfSequenceBase to 3844.

        const auto expectedFrame = m_pseudoDataInterpreter->getFollowingFrameNumber (m_frameNumberOfSequenceBase, sequence);
        if (frameNumber != expectedFrame)
        {
            const auto expectedStartOfNextSequence = m_pseudoDataInterpreter->getFollowingFrameNumber (m_frameNumberOfSequenceBase, static_cast<uint16_t> (m_useCaseFrameCount));
            if (m_pseudoDataInterpreter->isGreaterFrame (frameNumber, expectedStartOfNextSequence))
            {
                // The frame number suggests that we're still in the same sequence, but the sequence
                // number doesn't match. This may be data corruption, but we don't drop any
                // already-received frames of the current set.
            }
            else
            {
                // The frame number says that we're in a new sequence.
                resetSequenceBase = true;
            }
        }

        if (sequence < m_sequenceNumberNextExpected)
        {
            // This isn't expected, maybe we've dropped exactly enough buffers for the frame counter
            // to wrap round. This edge case is unlikely, but checked to enable assumptions that
            // simplify the missed-buffer logic for m_sequenceNumberNextExpected.
            resetSequenceBase = true;
        }
    }

    if (resetSequenceBase)
    {
        // check for old frames from the old sequence
        // this would need std::lock_guard<std::mutex> lock (m_bufferLock);, but it's already locked earlier
        for (auto &collected : m_capturedFrames)
        {
            releaseSparseCollection (collected);
        }

        // Align the sequence base to the sequence containing the received frame.
        //
        // With mixed mode there can be complete frame groups even if we missed the zeroth frame of
        // the sequence.  Incomplete groups are detected and released either when we receive the
        // buffer that should contain the final frame of the group, or when we receive a buffer that
        // tells us that we missed the final frame of the group.
        m_frameNumberOfSequenceBase = m_pseudoDataInterpreter->frameNumberFwdDistance (frameNumber, sequence);
        m_sequenceBaseKnown = true;
    }
    else if (sequence != m_sequenceNumberNextExpected)
    {
        // Release any old frames that were waiting for the missed buffer.
        const auto searchEnd = m_bufferActionMap.find (sequence);
        for (auto x = m_bufferActionMap.find (m_sequenceNumberNextExpected); x != searchEnd; x++)
        {
            processReadyGroups (x->second.ready);
        }
    }

    const auto nextSequenceExpected = m_bufferActionMap.upper_bound (sequence);
    if (nextSequenceExpected == m_bufferActionMap.end())
    {
        // The expectation is that the next bufferCallback() will be for a new sequence,
        // triggering resetSequenceBase.
        m_sequenceNumberNextExpected = 0;
    }
    else
    {
        m_sequenceNumberNextExpected = nextSequenceExpected->first;
    }

    const auto &action = m_bufferActionMap.at (sequence);
    auto frameCount = action.mapping.size();

    holder->checkMaxFrameCount (frameCount);
    updateFrameNumber (frameNumber, frameCount);

    if (!m_pseudoDataInterpreter->supportsExposureFromPseudoData())
    {
        // The imager will change the reconfiguration index to signal that new settings are effective,
        // the new index will be received in the pseudodata of the first frame that it applies to.
        checkForReconfig (capturedReconfigIndex);
    }

    // The BufferHolder will be owned by the links in the wrappers, if and only if at least one
    // wrapper is created.  One option is to simply use a std::shared_ptr, but the speed test
    // shows that this adds overhead.  The ownership here is handled by removing ownership from
    // the unique_ptr when a wrapper is created.
    auto holderForLoop = holder.get();
    for (std::size_t i = 0; i < frameCount; i++)
    {
        for (const auto &mapFramesTo : action.mapping[i])
        {
            auto &destination = m_capturedFrames[mapFramesTo.group][mapFramesTo.index];
            if (destination)
            {
                LOG (WARN) << "Two frames mapped to the same destination: "
                           << uint32_t {m_pseudoDataInterpreter->getFrameNumber (*destination) }
                           << " and "
                           << uint32_t {m_pseudoDataInterpreter->getFrameNumber (holderForLoop->getPseudoData (i)) };
            }
            else
            {
                destination = holderForLoop->getWrapper (i);
                (void) holder.release();
            }
        }
    }
    // If no wrappers were created, release the buffer.
    holder.reset();

    processReadyGroups (action.ready);
}

void FrameCollectorBase::processReadyGroups (const decltype (BufferAction::ready) &ready)
{
    for (const auto group : ready)
    {
        auto &collected = m_capturedFrames[group];
        const auto &expectation = m_frameGroupList[group];
        if (collected.size() != expectation.sequence.size())
        {
            throw LogicError ("Internal frame collector assumptions wrong, unexpected size");
        }

        bool allAsExpected = true;
        for (std::size_t i = 0; i < collected.size(); i++)
        {
            if (collected[i] == nullptr || expectation.sequence[i] != m_pseudoDataInterpreter->getSequenceIndex (*collected[i]))
            {
                allAsExpected = false;
                break;
            }
        }

        if (allAsExpected)
        {
            // sequence is complete, move it to a vector that's safe after unlocking the mutex

            updateEyeSafetyErrorAndState (*collected.front());

            if (m_temperatureSensor)
            {
                updateTemperature (IMMEDIATE_TEMPERATURE_UPDATE);
            }
            if (m_psdTemperatureSensor)
            {
                try
                {
                    m_temperatureReading = m_psdTemperatureSensor->calcTemperature (collected);
                }
                catch (Exception &e)
                {
                    // There was an error reading out the temperature from the pseudo data (it is likely
                    // that the pseudo data interpreter used does not support it)
                    LOG (ERROR) << "Error while trying to read the temperature from pseudo data: " << e.what();
                    reportErrorToClient ("Error while trying to read the temperature from pseudo data");
                }
            }

            // First, create the CapturedUseCase
            // Timestamp, using the buffers' hardware timestamps if available
            std::chrono::microseconds timestamp;
            auto hardwareStart = static_cast<WrappedRawFrame *> (collected.front())->getTimeMicroseconds();
            auto hardwareStop = static_cast<WrappedRawFrame *> (collected.back())->getTimeMicroseconds();
            if (hardwareStart != 0u && hardwareStop != 0u)
            {
                std::chrono::duration<uint64_t, std::micro> hardwareAverage { (hardwareStart + hardwareStop) / 2};
                timestamp = std::chrono::duration_cast<std::chrono::microseconds> (hardwareAverage);
            }
            else
            {
                timestamp = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
            }

            if (m_pseudoDataInterpreter->supportsExposureFromPseudoData())
            {
                for (auto expoSeqVals : expectation.exposeq)
                {
                    auto expoTime = m_pseudoDataInterpreter->getExposureTime (*collected[expoSeqVals.rawFrameIdx], expoSeqVals.modulationFrequency);
                    m_exposureTimes[expoSeqVals.exposureGroupIdx] = expoTime;
                }
            }

            std::unique_ptr<CapturedUseCase> cuc{ new CapturedUseCase{ m_pseudoDataInterpreter.get(), m_temperatureReading, timestamp, m_exposureTimes } };

            auto callback = new CallbackData();
            updateStats (0, collected.size());
            std::vector<ICapturedRawFrame *> tempFrames (collected.size(), nullptr);
            tempFrames.swap (collected);
            callback->frames.swap (tempFrames);
            callback->definition = m_useCaseDefinition;
            callback->streamId = expectation.streamId;
            callback->capturedCase = std::move (cuc);
            m_conveyanceQueue.push (callback);
            m_conveyanceCV.notify_one();
        }
        else
        {
            releaseSparseCollection (collected);
        }
    }
}

void FrameCollectorBase::releaseCapturedFrame (ICapturedRawFrame *frame)
{
    auto wrapper = dynamic_cast<WrappedRawFrame *> (frame);
    if (wrapper == nullptr)
    {
        throw LogicError ("Trying to release a CapturedRawFrame that wasn't sent by this FrameCollector");
    }
    auto holder = wrapper->getHolder();
    auto wasLastWrapper = holder->releaseWrapper (wrapper);
    if (wasLastWrapper)
    {
        // The buffer is released by BufferHolder's destructor
        delete holder;
    }
}

void FrameCollectorBase::releaseCapturedFrames (std::vector<ICapturedRawFrame *> frames)
{
    for (auto frame : frames)
    {
        releaseCapturedFrame (frame);
    }
}

void FrameCollectorBase::releaseSparseCollection (std::vector<ICapturedRawFrame *> &frames)
{
    std::size_t dropped = 0;
    for (auto &frame : frames)
    {
        if (frame != nullptr)
        {
            dropped++;
            releaseCapturedFrame (frame);
            frame = nullptr;
        }
    }
    updateStats (dropped, 0u);
}

void FrameCollectorBase::updateEyeSafetyErrorAndState (const royale::common::ICapturedRawFrame &frame)
{
    uint32_t eyeError = 0u;
    m_pseudoDataInterpreter->getEyeSafetyError (frame, eyeError);
    if (eyeError != 0u)
    {
        m_eventForwarder.event<event::EventEyeSafety> (eyeError);
    }
}
