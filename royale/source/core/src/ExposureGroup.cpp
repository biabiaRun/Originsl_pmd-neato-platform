/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <usecase/ExposureGroup.hpp>

using namespace royale::usecase;

bool ExposureGroup::operator== (const ExposureGroup &rhs) const
{
    if (m_name == rhs.m_name &&
            m_exposureLimits == rhs.m_exposureLimits &&
            m_exposureTime == rhs.m_exposureTime)
    {
        return true;
    }
    return false;
}

bool ExposureGroup::operator!= (const ExposureGroup &rhs) const
{
    return !operator== (rhs);
}
