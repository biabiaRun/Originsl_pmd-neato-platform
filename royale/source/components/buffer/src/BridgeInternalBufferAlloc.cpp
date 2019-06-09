/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <common/RoyaleLogger.hpp>

#include <buffer/BridgeInternalBufferAlloc.hpp>
#include <buffer/SimpleCapturedBuffer.hpp>

#include <common/MakeUnique.hpp>

#include <chrono>
#include <cstring>
#include <iterator>
#include <thread>
#include <vector>

using namespace royale::common;
using namespace royale::buffer;

BridgeInternalBufferAlloc::BridgeInternalBufferAlloc () :
    m_currentBuffers(),
    m_queuedBuffers(),
    m_bufferChangeInProgress (false),
    m_lock(),
    m_sleepCV(),
    m_captureListener (nullptr)
{
}

BridgeInternalBufferAlloc::~BridgeInternalBufferAlloc ()
{
    if (m_captureListener)
    {
        m_captureListener->releaseAllBuffers();
    }

    // All buffers will be automatically deleted when m_currentBuffers is destroyed after this
    // function returns.  They should all be queued, with none being held by the already-destroyed
    // subclass' acquisition function and none held by the capture listener.
    //
    // If any of the buffers aren't queued then something else is still claiming ownership, and may
    // still access them.  This would indicate a bug, but we leave the still-accessible buffers
    // still allocated to hopefully avoid memory corruption.
    if (m_queuedBuffers.size() != m_currentBuffers.size())
    {
        LOG (ERROR) << "Destroying BridgeInternalBufferAlloc with some buffers still dequeued";
        for (auto &buffer : m_currentBuffers)
        {
            buffer.release();
        }
        for (auto buffer : m_queuedBuffers)
        {
            delete buffer;
        }
    }
}

void BridgeInternalBufferAlloc::createAndQueueInternalBuffers (std::size_t count, size_t size, size_t pixelOffset, size_t pixelCount)
{
    // the allocation could be done before taking the lock, but when there's no buffers the only
    // thread that's likely to block on the lock is the acquisition thread, which will block until
    // the buffers are allocated anyway
    std::lock_guard<std::mutex> lock (m_lock);
    if (!m_currentBuffers.empty())
    {
        LOG (ERROR) << "createAndQueueInternalBuffers called with already-allocated buffers";
        throw LogicError ("createAndQueueInternalBuffers called, but buffers are already allocated");
    }

    LOG (DEBUG) << "Allocating " << count << " buffers";

    std::vector<std::unique_ptr<OffsetBasedCapturedBuffer> > handles;
    for (std::size_t i = 0; i < count; i++)
    {
        auto buffer = makeUnique<SimpleCapturedBuffer> (size, pixelOffset, pixelCount);
        handles.push_back (std::move (buffer));
    }

    m_currentBuffers = std::move (handles);

    for (auto &buffer : m_currentBuffers)
    {
        queueBufferLocked (buffer.get());
    }
}

void BridgeInternalBufferAlloc::queueBuffer (royale::hal::ICapturedBuffer *buffer)
{
    std::lock_guard<std::mutex> lock (m_lock);
    auto sbuffer = dynamic_cast<OffsetBasedCapturedBuffer *> (buffer);
    if (sbuffer == nullptr)
    {
        throw LogicError ("Queueing a buffer that wasn't allocated by this class");
    }
    queueBufferLocked (sbuffer);
}

void BridgeInternalBufferAlloc::queueBufferLocked (OffsetBasedCapturedBuffer *buffer)
{
    for (auto queued : m_queuedBuffers)
    {
        if (queued == buffer)
        {
            LOG (ERROR) << "Queueing already-queued buffer";
            throw LogicError ("Queueing already-queued buffer");
        }
    }
    m_queuedBuffers.push_back (buffer);

    if (m_bufferChangeInProgress)
    {
        tryDeallocBuffersLocked();
    }
    else if (m_queuedBuffers.size() == 1)
    {
        // dequeueInternalBuffer() might be waiting for the one that has just been queued
        m_sleepCV.notify_all();
    }
}

OffsetBasedCapturedBuffer *BridgeInternalBufferAlloc::dequeueInternalBuffer()
{
    std::unique_lock<std::mutex> lock (m_lock);
    auto blockWhile = [this]
    {
        if (! m_queuedBuffers.empty())
        {
            // there's a buffer available
            return true;
        }
        if (m_bufferChangeInProgress)
        {
            // return nullptr
            return true;
        }
        return !shouldBlockForDequeue();
    };
    m_sleepCV.wait (lock, blockWhile);

    if (m_queuedBuffers.empty() || m_bufferChangeInProgress)
    {
        return nullptr;
    }

    OffsetBasedCapturedBuffer *pHandle = m_queuedBuffers.back();
    m_queuedBuffers.pop_back();
    return pHandle;
}

bool BridgeInternalBufferAlloc::shouldBlockForDequeue()
{
    return false;
}

void BridgeInternalBufferAlloc::unblockDequeueThread()
{
    std::unique_lock<std::mutex> lock (m_lock);
    m_sleepCV.notify_all();
}

void BridgeInternalBufferAlloc::waitCaptureBufferDealloc()
{
    // Scopes for two different locks, to call releaseAllBuffers() in a state where queueBuffer
    // won't block.  This shouldn't be necessary (the framecollectors are supposed to release the
    // buffers before and during executeUseCase), but the listener is not necessarily a
    // framecollector.
    {
        std::unique_lock<std::mutex> lock (m_lock);
        m_bufferChangeInProgress = true;
    }
    {
        std::unique_lock<std::mutex> lock (m_changeListenerLock);
        if (m_captureListener)
        {
            m_captureListener->releaseAllBuffers();
        }
    }

    std::unique_lock<std::mutex> lock (m_lock);
    tryDeallocBuffersLocked();
    m_sleepCV.wait (lock, [this] { return !m_bufferChangeInProgress; });
}

void BridgeInternalBufferAlloc::tryDeallocBuffersLocked()
{
    if (m_queuedBuffers.size() != m_currentBuffers.size())
    {
        return;
    }

    m_currentBuffers.clear();
    m_queuedBuffers.clear();
    m_bufferChangeInProgress = false;
    m_sleepCV.notify_all();
}

void BridgeInternalBufferAlloc::setBufferCaptureListener (royale::hal::IBufferCaptureListener *listener)
{
    std::lock_guard<std::mutex> lock (m_changeListenerLock);
    m_captureListener = listener;
}

void BridgeInternalBufferAlloc::bufferCallback (royale::hal::ICapturedBuffer *buffer)
{
    std::lock_guard<std::mutex> lock (m_changeListenerLock);
    if (m_captureListener && ! m_bufferChangeInProgress)
    {
        m_captureListener->bufferCallback (buffer);
    }
    else
    {
        queueBuffer (buffer);
    }
}
