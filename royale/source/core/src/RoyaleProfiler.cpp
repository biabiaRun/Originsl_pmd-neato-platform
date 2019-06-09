/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <cstdint>
#include <common/RoyaleProfiler.hpp>

using namespace royale::common;

static const int64_t invalidData = (int64_t) (0x8000000000000000);

/**
    Ctor calls start implicitly
**/
RoyaleProfiler::RoyaleProfiler()
{
    start();    // automatically call start to use it during creation
}

/**
    Alias definition to restart();
**/
int64_t RoyaleProfiler::reset()
{
    return restart();
}

/**
    Defines the used instance as invalid.
    An invalid object can be checked with isValid(). Calculations of concerning
    functions of an invalidated timer may produce bizarre results.
**/
void RoyaleProfiler::invalidate()
{
    m_tpbase    = invalidData;
    m_tpdetail  = invalidData;
}

/**
    Returns true is the timer is still valid and wasn't invalidated.
**/
bool RoyaleProfiler::isValid() const
{
    return (m_tpbase != invalidData) && (m_tpdetail != invalidData);
}

bool RoyaleProfiler::hasExpired (int64_t timeout) const
{
    // if timeout is -1, (uint64_t)(timeout) is MAX -> will never expire
    return (uint64_t) (elapsed()) > (uint64_t) (timeout);
}