/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <usb/bridge/BridgeAmundsenCommon.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/IntegerMath.hpp>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * A multiple-iterator circular buffer, tailored for BridgeAmundsenCommon, and separated
             * from it so that a unit test can include it. No other users are anticipated.
             *
             * The "unprepared" and "prepared" iterators return pointers to areas of
             * AMUNDSEN_STRIDE_SIZE bytes, and are used for asynchronous I/O based on queueing
             * buffers for I/O and then waiting for the system to fill them in the order that they
             * were queued.  The "unprepared" iterator is checked with hasUnprepared, its pointer is
             * read with firstUnprepared, and the iterator is incremented with setPrepared.  The
             * "prepared" iterator is checked with hasPrepared, its pointer is read with
             * firstPrepared, and the iterator is incremented with setUsed.
             *
             * The remaining functions use the fact that the underlying buffer is a contiguous
             * circular buffer, and so the received data is in at most two continuous areas.
             * BridgeAmundsenCommon calls setCurrentBufferIsFrameStart() when it receives a payload
             * that is expected to be the first payload of the image.
             *
             * When the last payload is received then it with be in the area from startOfFrame()
             * until currentBuffer() + AMUNDSEN_STRIDE_SIZE. If this is a single contiguous then
             * isWrapped() will return false, otherwise isWrapped() returns true and the data is
             * split in to two contiguous areas which are wrapped around the join in the circular
             * buffer (startOfFrame() to end() and start() to currentBuffer() +
             * AMUNDSEN_STRIDE_SIZE.
             *
             * Multiple payloads are then converted with BufferUtils::copyAndNormalizeStrides, which
             * requires the addresses and sizes to read.
             */
            class AmundsenCircularBuffer
            {
            public:
                /**
                 * Constructor, the size_t argument is in strides, not bytes.
                 */
                explicit AmundsenCircularBuffer (std::size_t strides) :
                    m_receiveBuffer (strides * BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE),
                    m_receiveBufferEnd (m_receiveBuffer.data() + m_receiveBuffer.size()),
                    m_firstUnpreparedPayload (m_receiveBuffer.data()),
                    m_firstPrepared (m_receiveBuffer.data()),
                    m_currentBuffer (nullptr),
                    m_startOfFrame (nullptr),
                    m_strideCount (strides),
                    m_countPrepared (0)
                {
                }

                /**
                 * The wrap point, the first byte of the underlying buffer.
                 */
                uint8_t *start()
                {
                    return m_receiveBuffer.data();
                }

                /**
                 * The wrap point, one beyond final byte of the underlying buffer.
                 */
                uint8_t *end()
                {
                    return m_receiveBufferEnd;
                }

                bool hasUnprepared()
                {
                    return m_countPrepared < m_strideCount;
                }

                uint8_t *firstUnprepared()
                {
                    if (!hasUnprepared())
                    {
                        throw royale::common::LogicError ("No unprepared buffers");
                    }
                    return m_firstUnpreparedPayload;
                }

                /**
                 * Move the firstUnprepared buffer to the prepared queue.
                 */
                void setPrepared()
                {
                    m_countPrepared++;
                    royale::common::circularBufferIncrement (m_firstUnpreparedPayload, start(), end(), BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE);
                }

                /**
                 * True if setPrepared() has been called more ofter than setUsed().
                 */
                bool hasPrepared()
                {
                    return m_countPrepared != 0;
                }

                uint8_t *firstPrepared()
                {
                    if (!hasPrepared())
                    {
                        throw royale::common::LogicError ("No prepared buffers");
                    }
                    m_currentBuffer = m_firstPrepared;
                    return m_firstPrepared;
                }

                /**
                 * Move the firstPrepared buffer to the unprepared queue, and change getCurrentBuffer() to
                 * return this buffer.
                 */
                void setUsed()
                {
                    m_countPrepared--;
                    royale::common::circularBufferIncrement (m_firstPrepared, start(), end(), BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE);
                }

                /**
                 * Returns the same as the last call to firstPrepared(), even if setUsed() has been called
                 * since then.
                 */
                uint8_t *currentBuffer()
                {
                    return m_currentBuffer;
                }

                /**
                 * The data for the frame (as in, data for one ICapturedBuffer) starts at this point in the
                 * buffer, but may wrap round.  When set to nullpointer, we're waiting for the start of the
                 * next frame (as in, start of the data for the next ICapturedBuffer).
                 */
                uint8_t *startOfFrame()
                {
                    return m_startOfFrame;
                }

                void setCurrentBufferIsFrameStart()
                {
                    m_startOfFrame = m_currentBuffer;
                }

                void clearStartOfFrame()
                {
                    m_startOfFrame = nullptr;
                }

                /**
                 * Returns true if the data, starting when setCurrentBufferIsFrameStart() was called and
                 * ending with the currentBuffer(), is split by the end of the underlying buffer.
                 */
                bool isWrapped()
                {
                    return m_startOfFrame > m_currentBuffer;
                }

                std::size_t countUsedStridesExcludingCurrentBuffer()
                {
                    if (isWrapped())
                    {
                        return (m_receiveBufferEnd - m_startOfFrame + m_currentBuffer - start()) / BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE;
                    }
                    return (m_currentBuffer - m_startOfFrame) / BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE;
                }

            private:
                std::vector<uint8_t> m_receiveBuffer;
                uint8_t *const m_receiveBufferEnd;
                // Once we reach this point, we need to prepare more payloads
                uint8_t *m_firstUnpreparedPayload;
                // The next data capture will start at this point
                uint8_t *m_firstPrepared;
                uint8_t *m_currentBuffer;
                uint8_t *m_startOfFrame;
                const std::size_t m_strideCount;
                std::size_t m_countPrepared;
            };
        }
    }
}
