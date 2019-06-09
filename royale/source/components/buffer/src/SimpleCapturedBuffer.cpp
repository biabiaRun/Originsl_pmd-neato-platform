/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <buffer/SimpleCapturedBuffer.hpp>

using namespace royale::buffer;

SimpleCapturedBuffer::SimpleCapturedBuffer (std::size_t size, std::size_t pixelOffset, std::size_t pixelCount) :
    OffsetBasedCapturedBuffer {new uint8_t[size], size, pixelOffset, pixelCount, 0}
{
}

SimpleCapturedBuffer::~SimpleCapturedBuffer()
{
    delete[] getUnderlyingBuffer();
}
