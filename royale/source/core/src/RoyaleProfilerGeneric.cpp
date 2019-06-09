/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#if defined(__linux__) || defined(__APPLE__)
// ask for the latest POSIX, just in case
#define _POSIX_C_SOURCE 200809L

#include <sys/time.h>
#include <time.h>

#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <cstdint>
#include <ElapsedTimer.hpp>

using namespace royale::common;

enum
{
    SECS_PER_DAY    = 86400,
    MSECS_PER_DAY   = 86400000,
    SECS_PER_HOUR   = 3600,
    MSECS_PER_HOUR  = 3600000,
    SECS_PER_MIN    = 60,
    MSECS_PER_MIN   = 60000,
    MSECS_PER_SEC   = 1000,
    JULIAN_DAY_FOR_EPOCH = 2440588 // result of julianDayFromDate(1970, 1, 1)
};

static inline int64_t floordiv (int64_t a, int64_t b)
{
    return (a - (a < 0 ? b - 1 : 0)) / b;
}

static inline int64_t julianDayFromDate (int64_t year, int64_t month, int64_t day)
{
    // Adjust for no year 0
    if (year < 0)
    {
        ++year;
    }

    /*
    * Math from The Calendar FAQ at http://www.tondering.dk/claus/cal/julperiod.php
    * This formula is correct for all julian days, when using mathematical integer
    * division (round to negative infinity), not c++11 integer division (round to zero)
    */
    int64_t a   = floordiv (int64_t (14 - month), 12);
    int64_t y   = (int64_t) year + 4800 - a;
    int64_t m   = month + 12 * a - 3;
    return day + floordiv (int64_t (153 * m + 2), 5) + 365 * y + floordiv (y, 4) - floordiv (y, 100) + floordiv (y, 400) - 32045;
}

#if defined(_WIN32) || defined(_WIN64)
static inline int64_t msSinceEpoch()
{
    SYSTEMTIME st;
    memset (&st, 0, sizeof (SYSTEMTIME));
    GetSystemTime (&st);

    uint64_t mSecsTotal = 0;

    // Number of milliseconds for todays daytime referring to today's hour, minute, second and millisecond
    mSecsTotal += st.wHour         * MSECS_PER_HOUR;
    mSecsTotal += st.wMinute       * MSECS_PER_MIN;
    mSecsTotal += st.wSecond       * MSECS_PER_MIN;
    mSecsTotal += st.wMilliseconds * MSECS_PER_MIN;

    // Add julianDay to today's millisecond value and subtract JulianDay since epoch to get milliseconds since EPOCH
    mSecsTotal += int64_t (julianDayFromDate (st.wYear, st.wMonth, st.wDay) - JULIAN_DAY_FOR_EPOCH) * int64_t (MSECS_PER_DAY);

    return mSecsTotal;
}
#elif defined(__linux__) || defined(__APPLE__)
static inline int64_t msSinceEpoch()
{
    // posix compliant system
    // we have milliseconds
    struct timeval tv;
    gettimeofday (&tv, 0);
    return int64_t (tv.tv_sec) * int64_t (MSECS_PER_SEC) + tv.tv_usec / 1000;
}
#else
#error "Which system is that? That combination does not work."
#endif

RoyaleProfiler::ClockType RoyaleProfiler::clockType()
{
    return SystemTime;
}

bool RoyaleProfiler::isMonotonic()
{
    return false;
}

void RoyaleProfiler::start()
{
    restart();
}

int64_t RoyaleProfiler::restart()
{
    qint64 old  = m_tpbase;
    m_tpbase    = msSinceEpoch();
    m_tpdetail  = 0;
    return m_tpbase - old;
}

int64_t RoyaleProfiler::nsecsElapsed() const
{
    return msecsElapsed() * 1000000;
}

int64_t RoyaleProfiler::usecsElapsed() const
{
    return msecsElapsed() * 1000;
}

int64_t RoyaleProfiler::msecsElapsed() const
{
    return mSecsSinceEpoch() - m_tpbase;
}

int64_t RoyaleProfiler::secsElapsed() const
{
    return (mSecsSinceEpoch() - m_tpbase) / 1000;
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
    return m_tpbase * 1000000;
}

int64_t RoyaleProfiler::usecs() const
{
    return m_tpbase * 1000;
}

int64_t RoyaleProfiler::msecs() const
{
    return m_tpbase;
}

int64_t RoyaleProfiler::msecsTo (const RoyaleProfiler &other) const
{
    return other.m_tpbase - m_tpbase;
}

int64_t RoyaleProfiler::secsTo (const RoyaleProfiler &other) const
{
    return msecsTo (other) / 1000;
}

bool royale::common::operator< (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    return v1.m_tpbase < v2.m_tpbase
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