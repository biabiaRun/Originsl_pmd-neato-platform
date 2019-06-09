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
#include <string.h>
#include <atomic>

#ifdef _POSIX_MONOTONIC_CLOCK
#undef _POSIX_MONOTONIC_CLOCK
#endif
#define _POSIX_MONOTONIC_CLOCK -1

#include <common/RoyaleProfiler.hpp>

using namespace royale::common;

static const int64_t adjustValue = 1000 * 1000ull;

#ifndef CLOCK_REALTIME
#  define CLOCK_REALTIME 0
static inline void clockGettime (int, struct timespec *ts)
{
    // support clockGettime with gettimeofday
    struct timeval tv;
    gettimeofday (&tv, 0);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
}

#  ifdef _POSIX_MONOTONIC_CLOCK
#    undef _POSIX_MONOTONIC_CLOCK
#    define _POSIX_MONOTONIC_CLOCK -1
#  endif
#else
static inline void clockGettime (clockid_t clock, struct timespec *ts)
{
    clock_gettime (clock, ts);  // system function!
}
#endif

static int unixCheckClockType()
{
#if (_POSIX_MONOTONIC_CLOCK-0 == 0) && defined(_SC_MONOTONIC_CLOCK)
    // we need a value we can store in a clockid_t that isn't a valid clock
    // check if the valid ones are both non-negative or both non-positive
#  if CLOCK_MONOTONIC >= 0 && CLOCK_REALTIME >= 0
#    define IS_VALID_CLOCK(clock)    (clock >= 0)
#    define INVALID_CLOCK            -1
#  elif CLOCK_MONOTONIC <= 0 && CLOCK_REALTIME <= 0
#    define IS_VALID_CLOCK(clock)    (clock <= 0)
#    define INVALID_CLOCK            1
#  else
#    error "This system has weird values for CLOCK_MONOTONIC and CLOCK_REALTIME"
#  endif

    static std::atomic<int> clockToUse;
    atomic_init (&clockToUse, INVALID_CLOCK);

    int clock = atomic_load (&clockToUse);  // C++11 function!
    if (IS_VALID_CLOCK (clock))
    {
        return clock;
    }

    // detect if the system supports monotonic timers
    clock = sysconf (_SC_MONOTONIC_CLOCK) > 0 ? CLOCK_MONOTONIC : CLOCK_REALTIME;
    clockToUse.storeRelease (clock);
    return clock;

#  undef INVALID_CLOCK
#  undef IS_VALID_CLOCK
#elif (_POSIX_MONOTONIC_CLOCK-0) > 0
    return CLOCK_MONOTONIC;
#else
    return CLOCK_REALTIME;
#endif
}

static inline void doGettime (int64_t *sec, int64_t *nsecs)
{
    timespec ts;
    clockGettime (unixCheckClockType(), &ts);
    *sec    = ts.tv_sec;
    *nsecs  = ts.tv_nsec;
}

RoyaleProfiler::ClockType RoyaleProfiler::clockType()
{
    return unixCheckClockType() == CLOCK_REALTIME ? SystemTime : MonotonicClock;
}

bool RoyaleProfiler::isMonotonic()
{
    return clockType() == MonotonicClock;
}

void RoyaleProfiler::start()
{
    doGettime (&m_tpbase, &m_tpdetail);
}

int64_t RoyaleProfiler::restart()
{
    // Save old/current values since gettime() will overwrite them
    int64_t secs    = m_tpbase;
    int64_t nsecs   = m_tpdetail;
    doGettime (&m_tpbase, &m_tpdetail);

    // m_tp* holds the current time structure values now; substract the old values to get the difference
    secs    = m_tpbase - secs;
    nsecs   = m_tpdetail - nsecs;

    // Return the correct value of the difference
    return (secs * (int64_t) (1000)) + (nsecs / adjustValue);
}

int64_t RoyaleProfiler::nsecsElapsed() const
{
    int64_t sec, nsecs;
    doGettime (&sec, &nsecs);

    sec     = sec - m_tpbase;
    nsecs   = nsecs - m_tpdetail;

    return sec * (int64_t) (1000000000) + nsecs;
}

int64_t RoyaleProfiler::usecsElapsed() const
{
    return nsecsElapsed() / 1000;
}

int64_t RoyaleProfiler::msecsElapsed() const
{
    return nsecsElapsed() / adjustValue;
}

int64_t RoyaleProfiler::secsElapsed() const
{
    int64_t sec, nsecs;
    doGettime (&sec, &nsecs);

    sec = sec - m_tpbase;
    return sec;
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

int64_t RoyaleProfiler::msecs() const
{
    int64_t nsTime = m_tpbase * int64_t (1000) + m_tpdetail;
    return nsTime / adjustValue;
}

int64_t RoyaleProfiler::secs() const
{
    int64_t nsTime = m_tpbase * int64_t (1000) + m_tpdetail;
    return nsTime / (adjustValue * 1000);
}

int64_t RoyaleProfiler::usecs() const
{
    int64_t nsTime = m_tpbase * int64_t (1000) + m_tpdetail;
    return (nsTime * 1000) / adjustValue;
}

int64_t RoyaleProfiler::nsecs() const
{
    int64_t nsTime = m_tpbase * int64_t (1000) + m_tpdetail;
    return nsTime;
}

int64_t RoyaleProfiler::msecsTo (const RoyaleProfiler &other) const
{
    int64_t secs    = other.m_tpbase - m_tpbase;
    int64_t nsecs   = other.m_tpdetail - m_tpdetail;
    return (secs * (int64_t) (1000)) + (nsecs / adjustValue);
}

int64_t RoyaleProfiler::secsTo (const RoyaleProfiler &other) const
{
    return other.m_tpbase - m_tpbase;
}

bool royale::common::operator< (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    return v1.m_tpbase < v2.m_tpbase || (v1.m_tpbase == v2.m_tpbase && v1.m_tpdetail < v2.m_tpdetail);
}

RoyaleProfiler royale::common::operator+ (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    RoyaleProfiler profiler;
    profiler.m_tpbase = v1.m_tpbase + v2.m_tpbase;
    profiler.m_tpdetail = v1.m_tpdetail + v2.m_tpdetail;
    return profiler;
}

RoyaleProfiler royale::common::operator- (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    RoyaleProfiler profiler;
    profiler.m_tpbase = v1.m_tpbase - v2.m_tpbase;
    profiler.m_tpdetail = v1.m_tpdetail - v2.m_tpdetail;
    return profiler;
}
