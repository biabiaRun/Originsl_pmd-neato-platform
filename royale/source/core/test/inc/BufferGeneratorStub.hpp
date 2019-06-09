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

#include <collector/IFrameCollector.hpp>
#include <hal/ICapturedBuffer.hpp>
#include <hal/ITemperatureSensor.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <royale/Vector.hpp>

namespace royale
{
    namespace stub
    {
        namespace hal
        {
            /**
             * Simulates capturing data from an imager, including pseudodata for the
             * IPseudoDataInterpreter returned from the createInterpreter method.
             *
             * At the time of writing, this is only used for the FrameCollector's unit tests, and
             * only the pseudodata needed by the FrameCollector and its tests is generated.  The
             * IPseudoDataInterpreter returned by createInterpreter() throws NotImplemented if a
             * method for unsupported data is called.
             */
            class BufferGeneratorStub : public royale::hal::IBridgeDataReceiver
            {
            public:
                /**
                 * Duration (milliseconds) to wait for buffers to be returned.
                 *
                 * The FrameCollector is multithreaded, and should return buffers to the
                 * BufferGeneratorStub in a separate thread to the one that the tests use to check
                 * that getCounterBuffersQueued() == getCounterBuffersDequeued().
                 */
                static const int THREAD_WAIT_TIME = 10;

                /**
                 * Argument for setMaxBuffersInFlight.
                 */
                static const std::size_t MAX_AS_REQUESTED = 0;

                BufferGeneratorStub ();

                void setBufferCaptureListener (royale::hal::IBufferCaptureListener *collector) override;

                /**
                 * This makes any of the generate* methods sleep (for up to THREAD_WAIT_TIME)
                 * if maxInFlight buffers have already been dequeued and not yet requeued.  If there
                 * are still not enough buffers after that time then it throws.  For example, a
                 * limit of 18 means that an 18th buffer would be generated, but a 19th call to
                 * generate* would block.
                 *
                 * This is intended for tests where one thread is generating buffers in a loop, to
                 * ensure that the test doesn't fail due to the slowdown from allocating too much
                 * memory.
                 *
                 * The special value MAX_AS_REQUESTED will set it to the preferredBufferCount
                 * argument to executeUseCase(). This is the default.
                 *
                 * The special value std::numeric_limits<std::size_t>::max() means unlimited.
                 */
                void setMaxBuffersInFlight (std::size_t maxInFlight);

                /**
                 * When generating frames, if dropList.at (frameNumber) is true then the frame won't
                 * be generated. For superframes, the entire superframe is dropped if any frame
                 * within it is set to be dropped.
                 *
                 * Frames with numbers beyond the length of the vector will always be generated.
                 * Calling this with a zero-length vector will mean that no frames are dropped.
                 */
                void simulateFrameDrops (std::vector<bool> &&dropList);

                /**
                 * Configures the use case to the minimum size needed for the simulated pseudodata
                 * and a single line of image data.
                 *
                 * This reduces the time that the FrameCollector's unit tests take by about 50%.
                 */
                void reduceToMinimumImage (royale::usecase::UseCaseDefinition *useCase);

                /**
                 * Sets internal parameters according to the specified use case.
                 * This has to be called before every executeUseCase() call!
                 *
                 * The parameters will be used to set up several asserts in executeUseCase(),
                 * including an estimation of how many buffers the FrameCollector should request.
                 *
                 * @param useCase see IFrameCollector::executeUseCase's useCase
                 * @param bufferSizes see IFrameCollector::executeUseCase's measurementBlockSizes
                 */
                void configureFromUseCase (const royale::usecase::UseCaseDefinition *useCase,
                                           const royale::Vector<std::size_t> &bufferSizes);

                /**
                 * Throws if configureFromUseCase() has been called, but executeUseCase() hasn't.
                 */
                void checkExecuteFromUseCaseCalled () const;

                void queueBuffer (royale::hal::ICapturedBuffer *buffer) override;

                /**
                 * If set, the queueBuffer method will throws an exception.
                 *
                 * Bridges should probably avoid throwing from the queueBuffer this except for cases
                 * that will always be caught by minimal testing, as testing any other error would
                 * need to include replicating a race condition.
                 */
                void simulateExceptionInQueueBuffer (bool enable);

                /**
                 * Returns an IPseudoDataInterpreter matching the type of imager that this class
                 * simulates.  This IPseudoDataInterpreter throws NotImplemented from methods where
                 * the BufferGeneratorStub does not provide the necessary data.
                 */
                std::unique_ptr<royale::common::IPseudoDataInterpreter> createInterpreter();

                /**
                 * During simulation, push in numbers for the pseudodata, and then call bufferCallback.
                 *
                 * The pseudodata will have the frameCounter as the frame number.  The sequence
                 * number is calculated as (frameCounter % frames in use case), which matches the
                 * situation when the imager has started, has only run this use case, and hasn't
                 * generated enough frames for the frame counter to wrap round yet.
                 */
                void generateBufferCallback (uint16_t frameCounter);

                void generateBufferCallback (uint16_t frameCounter, uint16_t reconfigCounter);

                /**
                 * Create a superframe, push in numbers for the pseudodata, and then call bufferCallback.
                 *
                 * The sequence numbers will be 0, 1, ..., (frameCounter.size() - 1).
                 */
                void generateBufferCallback (const std::vector<uint16_t> &frameCounter);

                /**
                 * Create a superframe, push in numbers for the pseudodata, and then call bufferCallback.
                 *
                 * The timestamp is in the same time unit as ICapturedBuffer's getter function.
                 */
                void generateBufferCallback (const std::vector<uint16_t> &frameCounter, const std::vector<uint16_t> &sequenceIndex, const std::vector<uint16_t> &reconfigCounter, uint64_t timestamp = 0u);

                /**
                 * Create a superframe containing size frames, and then call bufferCallback.  The
                 * size argument is the number of frames to include in the superframe.
                 *
                 * The frame numbers will be frameCounter, frameCounter + 1, ..., up to (frameCounter + size - 1)
                 *
                 * The sequence numbers will be 0, 1, ..., size - 1.
                 */
                void generateSizeStart (std::size_t size, uint16_t frameCounter);

                /**
                 * Mixed mode support, generates multiple calls to bufferCallback.  Creates a series
                 * of superframes, one for each element in the size array.  The frame numbers and
                 * sequence numbers will continue monotonically across all of the generated
                 * callbacks.
                 *
                 * The frame numbers will be frameCounter, frameCounter + 1, ..., up to (frameCounter + total size - 1)
                 *
                 * The sequence numbers will be 0, 1, ..., total size - 1.
                 */
                void generateSizesStart (const royale::Vector<std::size_t> &size, uint16_t frameCounter);

                /** The number of calls to functions that queue buffers (generateBufferCallback) */
                int getCounterBuffersDequeued ();

                /** The number of calls to queueBuffer */
                int getCounterBuffersQueued ();

                /**
                 * Returns true if getCounterBuffersQueued and getCounterBuffersDequeued would return the
                 * same number.  But if they don't, waits up to THREAD_WAIT_TIME for the missing buffers to
                 * be returned.
                 */
                bool checkCounterBuffersBalance ();

                /**
                 * Returns the number of buffers that were requested in the similarly-named argument
                 * to executeUseCase().
                 */
                std::size_t getPreferredBufferCount ();

                /**
                 * A more accepting version of checkCounterBuffersBalance, this waits for up to
                 * THREAD_WAIT_TIME for buffers to be returned, but returns as soon as (and returns
                 * true if) it would be possible to allocate the aboutToAllocate number while still
                 * having maxInFlight or less buffers dequeued.
                 *
                 * The two arguments are passed instead of having a subtraction in the caller to
                 * prevent arithmetic wrap-round for zero or std::numeric_limits::max values.
                 *
                 * This is intended for tests where one thread is generating buffers, to ensure that
                 * the test doesn't fail due to the slowdown from allocating too much memory.
                 */
                bool waitBuffersRequeued (std::size_t maxInFlight, std::size_t aboutToAllocate);

                /**
                 * Make sure to call configureFromUseCase() before every executeUseCase() call!
                 */
                std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;

                // other IBridgeDataReceiver methods, all of them throw NotImplemented
                float getPeakTransferSpeed() override;
                void startCapture() override;
                void stopCapture() override;
                bool isConnected() const override;
                royale::Vector<royale::Pair<royale::String, royale::String>>getBridgeInfo() override;
                void setEventListener (royale::IEventListener *listener) override;

            private:
                royale::hal::IBufferCaptureListener *m_frameCollector;
                std::size_t m_useCaseFrameCount;
                std::size_t m_width;
                std::size_t m_height;
                /**
                 * The largest buffer that a real imager might generate, assuming that it's in
                 * superframe mode.  In mixed mode, this takes account of the frame groups.
                 */
                std::size_t m_maxSuperframeHeight;
                /**
                 * An optimistic (possibly lower than the actual minimum) number of frames required
                 * to completely double-buffer data received.  The real minimum may be higher;
                 * however it allows a sanity check with different logic to the real calculation in
                 * the frame collector.  This applies to both individual and superframe modes.
                 *
                 * See the comments in the .cpp file's configureFromUseCase().
                 */
                std::size_t m_minFramesToDoubleBuffer;
                /**
                 * An optimistic (see m_minFramesToDoubleBuffer) count of the numbers of buffers
                 * that would be needed, assuming the data is transmitted as superframes.
                 */
                std::size_t m_minSuperframesToDoubleBuffer;
                std::atomic<uint32_t> m_counterBuffersCreated;
                std::atomic<uint32_t> m_counterBuffersDeleted;
                /**
                 * See setMaxBuffersInFlight, however this variable has no special values. The
                 * MAX_AS_REQUESTED is implemented by setting m_usePreferredCountAsMaxInFlight.
                 */
                std::size_t m_maxBuffersInFlight;
                /** Config for simulateFrameDrops(), if empty then no frames are dropped. */
                std::vector<bool> m_simulateDrop;
                /** Set from the arguments of executeUseCase() */
                std::size_t m_preferredBufferCount;
                bool m_usePreferredCountAsMaxInFlight;
                /** Triggers the behavior documented for simulateExceptionInQueueBuffer() */
                bool m_simulateExceptionInQueueBuffer;
                std::mutex m_mutex;
                /** Notified each time a buffer is deleted, to wake checkCounterBuffersBalance */
                std::condition_variable m_cv;

                /** set in configureFromUseCase(), checked if true and set to false again in executeUseCase() */
                bool m_hasBeenConfigured;
            };
        }
    }
}
