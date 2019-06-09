/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <usecase/FrameGroup.hpp>

using namespace royale::usecase;

bool FrameGroup::operator== (const FrameGroup &rhs) const
{
    return m_frameSetIds == rhs.m_frameSetIds;
}

bool FrameGroup::operator!= (const FrameGroup &rhs) const
{
    return !operator== (rhs);
}

void FrameGroup::addFrameSet (const RawFrameSetId &id)
{
    m_frameSetIds.push_back (id);
}
