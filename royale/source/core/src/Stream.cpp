/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <usecase/Stream.hpp>

using namespace royale::usecase;

bool Stream::operator== (const Stream &rhs) const
{
    if (m_id == rhs.m_id &&
            m_frameGroups == rhs.m_frameGroups)
    {
        return true;
    }
    return false;
}

bool Stream::operator!= (const Stream &rhs) const
{
    return !operator== (rhs);
}

FrameGroup *Stream::createFrameGroup()
{
    m_frameGroups.emplace_back (FrameGroup());
    return &m_frameGroups.back();
}
