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
#include <windows.h>
#include <common/RoyaleProfiler.hpp>
#include <common/RoyaleLogger.hpp>

using namespace royale::common;

// WINAPI Library function pointer to get TickCounter
typedef ULONGLONG (WINAPI *PtrGetTickCount64) (void);

// Symbol to TickCount
static PtrGetTickCount64 symGetTickCount64 = 0;

// Result of QueryPerformanceFrequency, 0 indicates that the high resolution timer is unavailable
static uint64_t counterFrequency = 0;
static bool resolvedLibs = false;

static void resolveLibs()
{
    if (resolvedLibs)
    {
        return;
    }

    // Acquire the handle for kernel from windows
    HMODULE kernel32 = GetModuleHandleW (L"kernel32");
    if (!kernel32)
    {
        return;
    }

    // dynamically lookup symbol for "GetTickCount64" - gets the processor dependant system tick
    symGetTickCount64 = (PtrGetTickCount64) GetProcAddress (kernel32, "GetTickCount64");

    // Retrieve the number of high-resolution performance counter ticks per second; Windows library
    /*  Winbase.h (include Windows.h)
        BOOL WINAPI QueryPerformanceFrequency(
        _Out_ LARGE_INTEGER *lpFrequency
        );
    */
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency (&frequency))
    {
        counterFrequency = 0;
    }
    else
    {
        counterFrequency = frequency.QuadPart;      // Got Performancecounter
    }

    resolvedLibs = true;
}

static inline int64_t ticksToNs (int64_t ticks)
{
    // counterFrequency is the count of ticks per second acquired from the OS
    const int64_t expressNanoSeconds = 1000000000;

    if (counterFrequency > 0)       // We have a high resolution performance counter acquired
    {
        int64_t seconds         = ticks / counterFrequency;
        int64_t nanoSeconds     = (ticks - (seconds * counterFrequency)) * expressNanoSeconds / counterFrequency;

        return seconds * expressNanoSeconds + nanoSeconds;
    }
    else                            // We do not a high resolution performance counter
    {
        // GetTickCount(64) return milliseconds
        return ticks * 1000000;
    }
}

static inline uint64_t tickCount()
{
    resolveLibs();

    // This avoids a division by zero and disables the high performance counter if it's not available
    if (counterFrequency > 0)
    {
        LARGE_INTEGER counter;

        if (QueryPerformanceCounter (&counter))
        {
            return counter.QuadPart;
        }
        else
        {
            //LOG (WARN) << "QueryPerformanceCounter failed, although QueryPerformanceFrequency succeeded.";
            return 0;
        }
    }

    if (symGetTickCount64)
    {
        return symGetTickCount64();
    }

    static uint32_t highdword = 0;
    static uint32_t lastval = 0;
    uint32_t val = GetTickCount();

    if (val < lastval)
    {
        ++highdword;
    }

    lastval = val;
    return val | (uint64_t (highdword) << 32);
}

RoyaleProfiler::ClockType RoyaleProfiler::clockType()
{
    resolveLibs();

    if (counterFrequency > 0)
    {
        return PerformanceCounter;
    }
    else
    {
        return TickCounter;
    }
}

bool RoyaleProfiler::isMonotonic()
{
    return true;
}

void RoyaleProfiler::start()
{
    m_tpbase     = tickCount();
    m_tpdetail   = 0;
}

int64_t RoyaleProfiler::restart()
{
    int64_t lastTickPoint = m_tpbase;
    m_tpbase    = tickCount();
    m_tpdetail  = 0; // may probably be avoided; just for multiplatform support and to surpress warnings
    return ticksToNs (m_tpbase - lastTickPoint) / 1000000;       // refactor to milliseconds
}

int64_t RoyaleProfiler::nsecsElapsed() const
{
    return ticksToNs (tickCount() - m_tpbase);
}

int64_t RoyaleProfiler::usecsElapsed() const
{
    return nsecsElapsed() / 1000;        // refactor to microsecs (= ns / 10^3)
}

int64_t RoyaleProfiler::msecsElapsed() const
{
    return usecsElapsed() / 1000;        // refactor to milliseconds (= us / 10^6)
}

int64_t RoyaleProfiler::secsElapsed() const
{
    return elapsed() / 1000;        // refactor to seconds (= ms / 10^3)
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

int64_t RoyaleProfiler::secs() const
{
    return ticksToNs (m_tpbase) / 1000000000;      // refactor to seconds
}

int64_t RoyaleProfiler::msecs() const
{
    return ticksToNs (m_tpbase) / 1000000;       // refactor to milliseconds
}

int64_t RoyaleProfiler::usecs() const
{
    return ticksToNs (m_tpbase) / 1000;       // refactor to milliseconds
}

int64_t RoyaleProfiler::nsecs() const
{
    return ticksToNs (m_tpbase);       // refactor to milliseconds
}

int64_t RoyaleProfiler::msecsTo (const RoyaleProfiler &other) const
{
    int64_t difference = other.m_tpbase - m_tpbase;
    return ticksToNs (difference) / 1000000;       // refactor to milliseconds
}

int64_t RoyaleProfiler::secsTo (const RoyaleProfiler &other) const
{
    return msecsTo (other) / 1000;        // refactor to seconds
}

bool royale::common::operator< (const RoyaleProfiler &v1, const RoyaleProfiler &v2)
{
    return (v1.m_tpbase - v2.m_tpbase) < 0;
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
