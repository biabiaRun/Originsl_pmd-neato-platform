/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <buffer/OffsetBasedCapturedBuffer.hpp>

using namespace royale::buffer;

OffsetBasedCapturedBuffer::OffsetBasedCapturedBuffer (uint8_t *data, std::size_t size, std::size_t pixelOffset, std::size_t pixelCount, uint64_t timestamp) :
    m_data {data},
    m_size {size},
    m_pixelOffset {pixelOffset},
    m_pixelCount {pixelCount},
    m_timestamp {timestamp}
{
}

uint8_t *OffsetBasedCapturedBuffer::getUnderlyingBuffer()
{
    return m_data;
}

size_t OffsetBasedCapturedBuffer::getUnderlyingBufferSize()
{
    return m_size;
}

uint16_t *OffsetBasedCapturedBuffer::getPixelData()
{
    return reinterpret_cast<uint16_t *> (&m_data[m_pixelOffset]);
}

size_t OffsetBasedCapturedBuffer::getPixelCount()
{
    return m_pixelCount;
}

uint64_t OffsetBasedCapturedBuffer::getTimeMicroseconds()
{
    return m_timestamp;
}

void OffsetBasedCapturedBuffer::setTimeMicroseconds (uint64_t timestamp)
{
    m_timestamp = timestamp;
}
