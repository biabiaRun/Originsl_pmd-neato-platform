/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <record/RecordedRawFrame.hpp>

#include <string.h>

using namespace royale::record;

RecordedRawFrame::RecordedRawFrame (std::vector<uint16_t> &data, const uint16_t &columns)
{
    m_pseudoData = &data[0];
    m_imageData = &data[columns];
}

RecordedRawFrame::~RecordedRawFrame()
{
}

uint16_t *RecordedRawFrame::getImageData()
{
    return m_imageData;
}

const uint16_t *RecordedRawFrame::getPseudoData() const
{
    return m_pseudoData;
}
