/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <cstdint>
#include <vector>
#include <usecase/UseCaseDefinition.hpp>
#include <NarrowCast.hpp>
#include <common/ICapturedRawFrame.hpp>

namespace
{
    class SimpleRawFrame : public royale::common::ICapturedRawFrame
    {
    public:
        /**
         * Constructor.  Pixels include both the image data and the pseudo data.
         *
         * \param size number of uint16_t-sized pixels
         * \param pixelOffset where in the underlying data buffer the pixel data starts
         * \param imageOffset where in the underlying data buffer the UCD's image data starts
         * \param pseudoOffset where in the underlying data buffer the pseudo data starts
         */
        SimpleRawFrame (std::size_t size, std::size_t pixelOffset, std::size_t imageOffset, std::size_t pseudoOffset) :
            m_imageOffset (imageOffset),
            m_pseudoOffset (pseudoOffset)
        {
            m_data.resize (size);
        }

        uint16_t *getData()
        {
            return m_data.data();
        }

        uint16_t *getImageData() override
        {
            return &m_data[m_imageOffset];
        }
        const uint16_t *getPseudoData() const override
        {
            return &m_data[m_pseudoOffset];
        }
    private:
        std::vector<uint16_t> m_data;
        std::size_t m_imageOffset;
        std::size_t m_pseudoOffset;
    };
}

class CommonTestRecord
{
public:
    static uint16_t expectedValue (uint32_t idx, uint16_t frameNumber, uint16_t rawFrameNumber)
    {
        return static_cast<uint16_t> (idx * frameNumber + rawFrameNumber);
    }

    static void createFrames (std::vector<royale::common::ICapturedRawFrame *> &frames, royale::usecase::UseCaseDefinition &uc, uint16_t frameNumber)
    {
        uint16_t numCols, numRows;
        uc.getImage (numCols, numRows);

        numRows++;

        frames.resize (uc.getRawFrameCount());

        for (size_t i = 0; i < uc.getRawFrameCount(); ++i)
        {
            SimpleRawFrame *frame = new SimpleRawFrame (numCols * numRows, 0, numCols, 0);

            frames[i] = frame;

            uint16_t *buf = frame->getData();

            for (auto j = 0; j < numCols * numRows; ++j)
            {
                buf[j] = expectedValue (static_cast<uint16_t> (j), frameNumber, static_cast<uint16_t> (i));
            }
        }

    }
};
