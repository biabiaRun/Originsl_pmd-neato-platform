/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#define _POSIX_C_SOURCE 200809L
#include <cstdint>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <mach/mach_time.h>        // Apple timings library
#include <string.h>
#include <common/RoyaleProfiler.hpp>

using namespace royale::common;

static mach_timebase_info_data_t info = {0, 0};

static inline int64_t retrieveNs (int64_t cpuTime)
{
    if (info.denom == 0)
    {
        mach_timebase_info (&info);
    }
    return cpuTime * info.numer / info.denom;
}

RoyaleProfiler::ClockType RoyaleProfiler::clockType()
{
    return MachAbsoluteTime;        // indicates that we use mach absolute time; indicates furthermore that we are on APPLE
}

bool RoyaleProfiler::isMonotonic()
{
    return true;
}

void RoyaleProfiler::start()
{
    m_tpbase = mach_absolute_time();    // system function for precision clock
    return;
}

int64_t RoyaleProfiler::restart()
{
    int64_t lastTickPoint     = m_tpbase;
    m_tpbase                  = mach_absolute_time();
    return retrieveNs (m_tpbase - lastTickPoint) / 1000000;
}

int64_t RoyaleProfiler::nsecsElapsed() const
{
    uint64_t cpu_time = mach_absolute_time();
    return retrieveNs (cpu_time - m_tpbase);
}

int64_t RoyaleProfiler::usecsElapsed() const
{
    return nsecsElapsed() / 1000;
}

int64_t RoyaleProfiler::msecsElapsed() const
{
    uint64_t cpu_time = mach_absolute_time();
    return retrieveNs (cpu_time - m_tpbase) / 1000000;
}

int64_t RoyaleProfiler::secsElapsed() const
{
    return msecsElapsed() / 1000;
}

int64_t RoyaleProfiler::elapsed (const char *type) const
{
    if (type == nullptr || (strcmp (type, "ms") == 0))
    {
        return msecsElapsed();
    }

    else if (strcmp (type, "ns") == 0)
    {
        return nsecsElapsed();
    }

    else if (strcmp (type, "us") == 0)
    {
        return usecsElapsed();
    }

    else if (strcmp (type, "ms") == 0)
    {
        return msecsElapsed();
    }

    else if (strcmp (type, "s") == 0)
    {
        return secsElapsed();
    }

    return msecsElapsed();
}

int64_t RoyaleProfiler::nsecs() const
{
    return retrieveNs (m_tpbase);
}

int64_t RoyaleProfiler::usecs() const
{
    return retrieveNs (m_tpbase) / 1000;
}

int64_t RoyaleProfiler::msecs() const
{
    return retrieveNs (m_tpbase) / 1000000;
}

int64_t RoyaleProfiler::msecsTo (const RoyaleProfiler &other) const
{
    return retrieveNs (other.m_tpbase - m_tpbase) / 1000000;
}

int64_t RoyaleProfiler::secsTo (const RoyaleProfiler &other) const
{
    return msecsTo (other) / 1000;
}

bool royale::common::operator< (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    return v1.m_tpbase < v2.m_tpbase;
}

RoyaleProfiler royale::common::operator+ (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    RoyaleProfiler profiler;
    profiler.m_tpbase = v1.m_tpbase + v2.m_tpbase;
    return profiler;
}

RoyaleProfiler royale::common::operator- (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    RoyaleProfiler profiler;
    profiler.m_tpbase = v1.m_tpbase - v2.m_tpbase;
    return profiler;
}

