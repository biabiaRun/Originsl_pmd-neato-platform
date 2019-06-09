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

#include <hal/IBridgeDataReceiver.hpp>
#include <buffer/OffsetBasedCapturedBuffer.hpp>

#include <common/exceptions/LogicError.hpp>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>


namespace royale
{
    namespace buffer
    {
        /**
         * Common superclass for Bridges where the memory buffers are managed by the bridge.
         *
         * This provides a pool of OffsetBasedCapturedBuffers.
         */
        class BridgeInternalBufferAlloc : public royale::hal::IBridgeDataReceiver
        {
        protected:
            ROYALE_API BridgeInternalBufferAlloc ();

        public:
            // The public functions are only those accessible from IBridgeDataReceiver
            ROYALE_API ~BridgeInternalBufferAlloc () override;

            /**
             * Handled by this class, does not need to be implemented by the subclass.
             */
            ROYALE_API virtual void queueBuffer (royale::hal::ICapturedBuffer *buffer) override;

            /**
             * Handled by this class, does not need to be implemented by the subclass.
             */
            ROYALE_API void setBufferCaptureListener (royale::hal::IBufferCaptureListener *listener) override;

        protected:
            /**
             * Expected to be called during the subclass' executeUseCase(), creates and queues all
             * buffers for capturing images.
             *
             * If the buffer size needs to be changed, the subclass should call
             * waitCaptureBufferDealloc() before calling this function again.
             *
             * If double-buffering is required, the caller should pass a count that is twice the
             * number of raw frames in the use case definition.  When the Royale Core framework
             * calls IBridgeDataReceiver::executeUseCase, this has already been taken in to account.
             *
             * \param count how many buffers should be allocated
             * \param size the size in bytes of each normalized buffer, see SimpleCapturedBuffer
             * \param pixelOffset passed to SimpleCapturedBuffer's constructor
             * \param pixelCount passed to SimpleCapturedBuffer's constructor
             *
             * \throw bad_alloc on failure
             */
            ROYALE_API virtual void createAndQueueInternalBuffers (std::size_t count, std::size_t size, std::size_t pixelOffset, std::size_t pixelCount);

            /**
             * Blocks until all buffers have been released, the thread will be woken by the final
             * call from the processing thread to queueBuffer().  This is expected to be called in
             * the subclass' executeUseCase().
             *
             * All released buffers are deallocated.  If further images are to be captured, then
             * createAndQueueInternalBuffers() must be called afterwards.
             *
             * Buffers that are currently in the IBufferCaptureListener (or the processing code)
             * will not be deallocated until they are passed to queueBuffer() again, however this
             * method will block until this has happened.
             */
            ROYALE_API virtual void waitCaptureBufferDealloc();

            /**
             * This returns one of the buffers that queueBuffer has been called on.  If no buffers
             * are available it tests shouldBlockForDequeue(), and either returns nullptr or waits
             * for a buffer to be available.
             *
             * If it is blocking, calling unblockDequeueThread() will cause shouldBlockForDequeue()
             * to be tested again.
             *
             * The caller gains exclusive use of the buffer, and must either pass
             * ownership to Bridge::BufferCallback or requeue the buffer directly via
             * queueBuffer().
             */
            ROYALE_API royale::buffer::OffsetBasedCapturedBuffer *dequeueInternalBuffer();

            /**
             * If dequeueInternalBuffer is called and no buffers are available, this function will
             * be called.  If this returns false, then dequeueInternalBuffer will return nullptr.
             *
             * This can be called multiple times (for example, if a spurious lock wake triggers).
             * It will always be called in the thread that is calling dequeueInternalBuffer.
             *
             * The default implementation always returns false.
             */
            ROYALE_API virtual bool shouldBlockForDequeue();

            /**
             * Makes dequeueInternalBuffer retest shouldBlockForDequeue.
             */
            ROYALE_API void unblockDequeueThread();

            /**
             * Send a buffer to the IBufferCaptureListener.
             *
             * The caller must not hold any lock when sending buffers to the BufferCaptureListener
             * (or at least, must not hold any lock that could block that thread from calling any
             * Bridge, Imager or CameraModule function).
             *
             * If the last call to setBufferCaptureListener set the listener to nullptr, this will
             * simply call queueBuffer.
             */
            void bufferCallback (royale::hal::ICapturedBuffer *buffer);

        private:
            /**
             * All buffers allocated by the last call to createAndQueueInternalBuffers, whether
             * queued or not.  It's only safe to compare pointers, as the buffers may be
             * accessed by the processing thread.
             */
            std::vector<std::unique_ptr<royale::buffer::OffsetBasedCapturedBuffer> > m_currentBuffers;
            /**
             * Buffers available to dequeueBuffer().  All of these will also be in m_CurrentBuffers.
             */
            std::vector<royale::buffer::OffsetBasedCapturedBuffer *> m_queuedBuffers;
            /**
             * Set from waitCaptureBufferDealloc until the buffers are deallocated.
             */
            bool m_bufferChangeInProgress;
            /**
             * Must be held when adding or removing buffers from the vectors.
             */
            std::mutex m_lock;
            /**
             * waitCaptureBufferDealloc sleeps on this, queueBuffer notifies it
             */
            std::condition_variable m_sleepCV;

            /**
             * Internal implementation of queueBuffer, called with m_lock already held.
             */
            void queueBufferLocked (royale::buffer::OffsetBasedCapturedBuffer *buffer);

            /**
             * Checks if all buffers are currently queued, and if so deletes all of them and wakes
             * any thread that is blocked in waitCaptureBufferDealloc().
             *
             * Only called with m_lock already held, and m_BufferChangeInProgress true.
             */
            void tryDeallocBuffersLocked();

            /**
             * Must be held when accessing m_captureListener.
             */
            std::mutex m_changeListenerLock;

            /**
             * Where to send the buffers to
             */
            royale::hal::IBufferCaptureListener *m_captureListener;
        };
    }
}
