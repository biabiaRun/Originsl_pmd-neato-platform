/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <BufferGeneratorStub.hpp>

#include <collector/IFrameCollector.hpp>
#include <hal/ICapturedBuffer.hpp>
#include <hal/ITemperatureSensor.hpp>
#include <common/IntegerMath.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>
#include <common/PseudoDataTwelveBitCalculator.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/Timeout.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>
#include <royale/Vector.hpp>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;
using namespace royale::hal;
using namespace royale::stub::hal;

namespace
{
    /**
     * Simple implementation of an ICapturedBuffer.
     *
     * The name is from TestFrameCollectorIndividual.
     */
    class TfciCapturedBuffer : public ICapturedBuffer
    {
    public:
        TfciCapturedBuffer (std::size_t size, uint64_t timestamp) :
            m_data (size, 0u),
            m_timestamp (timestamp)
        {
        }

        uint16_t *getPixelData() override
        {
            return m_data.data();
        }

        std::size_t getPixelCount() override
        {
            return m_data.size();
        }

        uint64_t getTimeMicroseconds() override
        {
            return m_timestamp;
        }

    private:
        std::vector<uint16_t> m_data;
        uint64_t m_timestamp;
    };

    class PseudoDataHandler : public PseudoDataTwelveBitCalculator
    {
    private:
        // These numbers are arbitrary, this class encapsulates all access to the data.
        static const std::size_t FRAME_NUMBER_INDEX = 0;
        static const std::size_t SEQUENCE_NUMBER_INDEX = 1;
        static const std::size_t RECONFIG_INDEX = 2;
        static const std::size_t SMALLEST_WIDTH = 3;

        uint16_t narrow_cast_12 (uint16_t value) const
        {
            if (value & 0xf000)
            {
                throw OutOfBounds ("narrow_cast to 12-bit failed");
            }
            return value;
        }

    public:
        ~PseudoDataHandler() override = default;

        PseudoDataHandler *clone() override
        {
            return new PseudoDataHandler (*this);
        }

        /**
         * Returns an image size which would be sufficient to hold the full pseudodata, and which
         * considers the real hardware's constraint that this is a multiple of 16.
         */
        static uint16_t getSmallestSupportedWidth()
        {
            return static_cast<uint16_t> (roundUpToUnit (SMALLEST_WIDTH, 16));
        }

        uint16_t getFrameNumber (const royale::common::ICapturedRawFrame &frame) const override
        {
            return frame.getPseudoData() [FRAME_NUMBER_INDEX];
        }

        void setFrameNumber (uint16_t *data, uint16_t value) const
        {
            data[FRAME_NUMBER_INDEX] = narrow_cast_12 (value);
        }

        uint16_t getReconfigIndex (const royale::common::ICapturedRawFrame &frame) const override
        {
            return frame.getPseudoData() [RECONFIG_INDEX];
        }

        void setReconfigIndex (uint16_t *data, uint16_t value) const
        {
            data[RECONFIG_INDEX] = narrow_cast_12 (value);
        }

        uint16_t getSequenceIndex (const royale::common::ICapturedRawFrame &frame) const override
        {
            return frame.getPseudoData() [SEQUENCE_NUMBER_INDEX];
        }

        void setSequenceIndex (uint16_t *data, uint16_t value)
        {
            data[SEQUENCE_NUMBER_INDEX] = narrow_cast_12 (value);
        }

        uint8_t getBinning (const royale::common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint16_t getHorizontalSize (const royale::common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        uint16_t getVerticalSize (const royale::common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        float getImagerTemperature (const royale::common::ICapturedRawFrame &frame) const override
        {
            throw NotImplemented();
        }

        void getTemperatureRawValues (const royale::common::ICapturedRawFrame &frame,
                                      uint16_t &vRef1,
                                      uint16_t &vNtc1,
                                      uint16_t &vRef2,
                                      uint16_t &vNtc2,
                                      uint16_t &offset) const override
        {
            throw NotImplemented();
        }

        uint16_t getRequiredImageWidth() const override
        {
            return SMALLEST_WIDTH;
        }

        void getEyeSafetyError (const royale::common::ICapturedRawFrame &frame, uint32_t &eyeError) const override
        {
            eyeError = 0u;
        }

        bool supportsExposureFromPseudoData() const override
        {
            return false;
        }

        uint32_t getExposureTime (const royale::common::ICapturedRawFrame &frame, uint32_t modulationFrequency)  const override
        {
            throw royale::common::NotImplemented();
        }
    };
}

BufferGeneratorStub::BufferGeneratorStub () :
    m_frameCollector {nullptr},
    m_useCaseFrameCount {0},
    m_width {0},
    m_height {0},
    m_maxSuperframeHeight {0},
    m_minFramesToDoubleBuffer {0},
    m_minSuperframesToDoubleBuffer {0},
    m_counterBuffersCreated {0},
    m_counterBuffersDeleted {0},
    m_maxBuffersInFlight {std::numeric_limits<std::size_t>::max() },
    m_preferredBufferCount {0},
    m_usePreferredCountAsMaxInFlight {true},
    m_simulateExceptionInQueueBuffer {false},
    m_hasBeenConfigured {false}
{
}

std::unique_ptr<royale::common::IPseudoDataInterpreter> BufferGeneratorStub::createInterpreter()
{
    return makeUnique<PseudoDataHandler> ();
}

void BufferGeneratorStub::setBufferCaptureListener (IBufferCaptureListener *collector)
{
    m_frameCollector = collector;
}

void BufferGeneratorStub::setMaxBuffersInFlight (std::size_t maxInFlight)
{
    if (maxInFlight == MAX_AS_REQUESTED)
    {
        m_maxBuffersInFlight = m_preferredBufferCount;
        m_usePreferredCountAsMaxInFlight = true;
    }
    else
    {
        m_maxBuffersInFlight = maxInFlight;
        m_usePreferredCountAsMaxInFlight = false;
    }
}

void BufferGeneratorStub::simulateFrameDrops (std::vector<bool> &&dropList)
{
    m_simulateDrop = std::move (dropList);
}

void BufferGeneratorStub::reduceToMinimumImage (UseCaseDefinition *useCase)
{
    // One line of pseudodata, and one line of image data
    useCase->setImage (PseudoDataHandler::getSmallestSupportedWidth(), 2u);
}

void BufferGeneratorStub::configureFromUseCase (const UseCaseDefinition *useCase, const royale::Vector<std::size_t> &bufferSizes)
{
    if (m_hasBeenConfigured)
    {
        throw LogicError ("configureFromUseCase has been called twice without an executeUseCase call");
    }

    m_useCaseFrameCount = useCase->getRawFrameCount();
    uint16_t imageWidth;
    uint16_t imageHeight;
    useCase->getImage (imageWidth, imageHeight);
    m_width = imageWidth;
    // The height adds a line for the pseudodata
    m_height = imageHeight + 1;
    const std::size_t maxSuperframeHeightInFrames = *std::max_element (bufferSizes.cbegin(), bufferSizes.cend());
    m_maxSuperframeHeight = m_height * maxSuperframeHeightInFrames;

    // As noted in the .hpp file, the calculation for m_minFramesToDoubleBuffer is optimistic, to
    // sanity check by using a different calculation to the one in the FrameCollector.
    //
    // Currently it assumes:
    // * the buffers contain either individual frames or completely-filled superframes, there's no
    // other frames in the buffer or empty parts in the superframe.
    // * no duplicate frames.
    m_minFramesToDoubleBuffer = 0;
    m_minSuperframesToDoubleBuffer = 0;
    for (const auto streamId : useCase->getStreamIds())
    {
        std::size_t frameCount = 0;
        for (const auto rfsIdx : useCase->getRawFrameSetIndices (streamId, 0))
        {
            frameCount += useCase->getRawFrameSets().at (rfsIdx).countRawFrames ();
        }
        m_minFramesToDoubleBuffer += 2 * frameCount;

        // a very optimistic calculation of m_minSuperframesToDoubleBuffer
        const auto buffersPerGroup = double (useCase->getRawFrameSetIndices (streamId, 0).size()) / double (maxSuperframeHeightInFrames);
        m_minSuperframesToDoubleBuffer += 2 * static_cast<std::size_t> (std::ceil (buffersPerGroup));
    }
    m_hasBeenConfigured = true;
}

void BufferGeneratorStub::checkExecuteFromUseCaseCalled () const
{
    if (m_hasBeenConfigured)
    {
        throw LogicError ("executeUseCase hasn't been called since the configureFromUseCase call");
    }
}

void BufferGeneratorStub::queueBuffer (ICapturedBuffer *buffer)
{
    std::unique_lock<std::mutex> lock {m_mutex};
    m_counterBuffersDeleted++;
    delete buffer;
    m_cv.notify_all();
    if (m_simulateExceptionInQueueBuffer)
    {
        throw Exception ("simulateExceptionInQueueBuffer");
    }
}

void BufferGeneratorStub::simulateExceptionInQueueBuffer (bool enable)
{
    m_simulateExceptionInQueueBuffer = enable;
}

void BufferGeneratorStub::generateBufferCallback (uint16_t frameCounter)
{
    generateBufferCallback (frameCounter, 0);
}

void BufferGeneratorStub::generateBufferCallback (uint16_t frameCounter, uint16_t reconfigCounter)
{
    std::vector<uint16_t> frames { frameCounter };
    std::vector<uint16_t> sequence { narrow_cast<uint16_t> (frameCounter % m_useCaseFrameCount) };
    generateBufferCallback (frames, sequence, { reconfigCounter });
}

void BufferGeneratorStub::generateBufferCallback (const std::vector<uint16_t> &frameCounter)
{
    std::vector<uint16_t> sequenceIndex;
    std::vector<uint16_t> reconfigCounter;
    for (size_t i = 0; i < frameCounter.size(); i++)
    {
        sequenceIndex.push_back (narrow_cast<uint16_t> (i));
        reconfigCounter.push_back (0);
    }
    generateBufferCallback (frameCounter, sequenceIndex, reconfigCounter);
}

void BufferGeneratorStub::generateSizeStart (std::size_t size, uint16_t frameCounter)
{
    generateSizesStart ({size}, frameCounter);
}

void BufferGeneratorStub::generateSizesStart (const royale::Vector<std::size_t> &sizes, uint16_t frameCounter)
{
    size_t sequenceCounter = 0u;
    for (const auto size : sizes)
    {
        std::vector<uint16_t> frames;
        std::vector<uint16_t> sequenceIndex;
        std::vector<uint16_t> reconfigCounter;
        for (size_t i = 0; i < size; i++)
        {
            frames.push_back (narrow_cast<uint16_t> (frameCounter + i));
            sequenceIndex.push_back (narrow_cast<uint16_t> ( (sequenceCounter + i) % m_useCaseFrameCount));
            reconfigCounter.push_back (0);
        }
        generateBufferCallback (frames, sequenceIndex, reconfigCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + size);
        sequenceCounter += size;
    }
}

void BufferGeneratorStub::generateBufferCallback (const std::vector<uint16_t> &frameCounter,
        const std::vector<uint16_t> &sequenceIndex,
        const std::vector<uint16_t> &reconfigCounter,
        uint64_t timestamp)
{
    ASSERT_EQ (frameCounter.size(), sequenceIndex.size());
    ASSERT_EQ (frameCounter.size(), reconfigCounter.size());

    for (const auto i : frameCounter)
    {
        if (i < m_simulateDrop.size() && m_simulateDrop[i])
        {
            return;
        }
    }

    std::size_t pixelCount = m_width * m_height;
    std::size_t superPixelCount = pixelCount * frameCounter.size();

    std::unique_ptr<ICapturedBuffer> buffer {new TfciCapturedBuffer (superPixelCount, timestamp) };

    PseudoDataHandler pdh;
    for (std::size_t i = 0; i < frameCounter.size() ; i++)
    {
        // The convention used throughout Royale is that pseudodata is always the first line of the
        // data, and the hardware imagers are always configured for this.
        uint16_t *pseudoData = buffer->getPixelData() + i * pixelCount;
        pdh.setFrameNumber (pseudoData, frameCounter[i] & 0xfff);
        pdh.setSequenceIndex (pseudoData, sequenceIndex[i] & 0xfff);
        pdh.setReconfigIndex (pseudoData, reconfigCounter[i] & 0xfff);
    }

    if (!waitBuffersRequeued (m_maxBuffersInFlight, 1))
    {
        std::cout << "Buffers: " << m_counterBuffersCreated << " vs " << m_counterBuffersDeleted << std::endl;
    }

    m_counterBuffersCreated++;
    m_frameCollector->bufferCallback (buffer.release());
}

int BufferGeneratorStub::getCounterBuffersDequeued ()
{
    return m_counterBuffersCreated;
}

int BufferGeneratorStub::getCounterBuffersQueued ()
{
    return m_counterBuffersDeleted;
}

bool BufferGeneratorStub::waitBuffersRequeued (std::size_t maxInFlight, std::size_t aboutToAllocate)
{
    // The docs say that max() is the special value which means never block,
    // but any reasonably large value is limited by available memory, not
    // the actual number of buffers in flight.
    //
    // This avoids arithmetic overflow.
    if (maxInFlight >= std::numeric_limits<std::size_t>::max() / 2)
    {
        return true;
    }

    auto fitsInLimits = [this, maxInFlight, aboutToAllocate] { return m_counterBuffersCreated.load() + aboutToAllocate <= m_counterBuffersDeleted.load() + maxInFlight; };

    std::unique_lock<std::mutex> lock {m_mutex};
    std::chrono::milliseconds delay (THREAD_WAIT_TIME);
    m_cv.wait_for (lock, delay, fitsInLimits);
    return fitsInLimits();
}

bool BufferGeneratorStub::checkCounterBuffersBalance ()
{
    // Although waitBuffersRequeued (0) is similar, it would return true if the deleted counter was
    // greated than the created counter.
    std::unique_lock<std::mutex> lock {m_mutex};
    std::chrono::milliseconds delay (THREAD_WAIT_TIME);
    m_cv.wait_for (lock, delay, [this] { return m_counterBuffersCreated.load() == m_counterBuffersDeleted.load(); });
    return m_counterBuffersCreated == m_counterBuffersDeleted;
}

std::size_t BufferGeneratorStub::executeUseCase (int width, int height, std::size_t preferredBufferCount)
{
    if (!m_hasBeenConfigured)
    {
        throw LogicError ("configureFromUseCase has to be called before each executeUseCase call");
    }

    // check if parameters match with configured use case
    if (m_width != static_cast<std::size_t> (width))
    {
        throw LogicError ("configureFromUseCase has been called with a different UseCaseDefinition");
    }
    if (static_cast<std::size_t> (height) % m_height != 0)
    {
        throw LogicError ("FrameCollector requested a buffer size that isn't a multiple of the frame size");
    }
    const auto heightInFrames = static_cast<std::size_t> (height) / m_height;
    if (heightInFrames == 1)
    {
        // the frame collector expects individual frames
        if (preferredBufferCount > 2 * m_useCaseFrameCount)
        {
            // this applies to both non-mixed and mixed mode
            throw LogicError ("FrameCollector requested too many buffers");
        }
    }
    else
    {
        // the frame collector expects superframes
        if (m_maxSuperframeHeight != static_cast<std::size_t> (height))
        {
            throw LogicError ("Wrong buffer size");
        }
        if (preferredBufferCount < m_minSuperframesToDoubleBuffer)
        {
            throw LogicError ("FrameCollector requested too few superframe buffers");
        }
    }

    if (heightInFrames * preferredBufferCount < m_minFramesToDoubleBuffer)
    {
        throw LogicError ("FrameCollector requested too few frames to double-buffer");
    }
    else if (heightInFrames * preferredBufferCount >= 2 * m_minFramesToDoubleBuffer)
    {
        // This might be a failure of the m_minFramesToDoubleBuffer calculation and not an error in
        // the FrameCollector, but if something triggers it then we should check why so much memory
        // is needed.
        throw LogicError ("FrameCollector requested enough frames to quadruple-buffer");
    }

    m_preferredBufferCount = preferredBufferCount;
    if (m_usePreferredCountAsMaxInFlight)
    {
        m_maxBuffersInFlight = preferredBufferCount;
    }
    else if (preferredBufferCount > m_maxBuffersInFlight)
    {
        // Tell the frame collector that it has a different number than requested. At the time of
        // writing (Royale v3.23), this will only cause a warning message to be printed.
        preferredBufferCount = m_maxBuffersInFlight;
    }

    m_hasBeenConfigured = false;

    return preferredBufferCount;
}

float BufferGeneratorStub::getPeakTransferSpeed()
{
    throw royale::common::NotImplemented();
}

void BufferGeneratorStub::startCapture()
{
    throw royale::common::NotImplemented();
}

void BufferGeneratorStub::stopCapture()
{
    throw royale::common::NotImplemented();
}

bool BufferGeneratorStub::isConnected() const
{
    throw royale::common::NotImplemented();
}

royale::Vector<royale::Pair<royale::String, royale::String>>BufferGeneratorStub::getBridgeInfo()
{
    throw royale::common::NotImplemented();
}

void BufferGeneratorStub::setEventListener (royale::IEventListener *listener)
{
    throw royale::common::NotImplemented();
}
