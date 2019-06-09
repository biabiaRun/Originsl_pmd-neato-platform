/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

namespace royale
{
    namespace common
    {
        class RoyaleProfiler
        {
        public:
            /// Enum definitions for indication of used clock type - this is however system dependent and automatically determined
            enum ClockType
            {
                SystemTime,
                MonotonicClock,
                TickCounter,
                MachAbsoluteTime,
                PerformanceCounter
            };

            /// C'tor calls start() implicitly to allow direct comparisons while creation of Objects
            ROYALE_API RoyaleProfiler();

            /// Returns the clockType (depending on the enum values)
            static ROYALE_API ClockType clockType();


            static ROYALE_API bool isMonotonic();

            /** Starts the Timer and sets the startmark to m_tpbase / m_tpdetail.
                The difference between a certain point in time and the start time may
                later be determined with elapsed()
            **/
            ROYALE_API void start();

            /** Restarts the Timer and sets the startmark to m_tpbase / m_tpdetail again.
                The difference between the point of time when restart() was called and start()
                is returned - in the unit "ms"
            **/
            ROYALE_API int64_t restart();

            /** Reset is an alias definition to restart();
                they have an equal meaning
            **/
            ROYALE_API int64_t reset();

            /** Invalidates a Profiler; used if it is not planned to use this profile anymore
                in the current tree - not planned to be revoked; if once invalidated you cannot
                validate the object again
            **/
            ROYALE_API void invalidate();

            /** To ensure that the Profiler is not invalidated
            **/
            ROYALE_API bool isValid() const;

            /** Returns the elapsed time in nano seconds since start() was called or since the creation of the object
                if start was not called explicitly
            **/
            ROYALE_API int64_t nsecsElapsed() const;

            /** Returns the elapsed time in micro seconds since start() was called or since the creation of the object
                if start was not called explicitly
            **/
            ROYALE_API int64_t usecsElapsed() const;

            /** Returns the elapsed time in milli seconds since start() was called or since the creation of the object
                if start was not called explicitly
            **/
            ROYALE_API int64_t msecsElapsed() const;

            /** Returns the elapsed time in seconds since start() was called or since the creation of the object
                if start was not called explicitly
            **/
            ROYALE_API int64_t secsElapsed() const;

            /**
                Accepts following types:
                 "s"  - for seconds
                 "ms" - for milliseconds
                 "us" - for microseconds
                 "ns" - for nanoseconds
                  - defaults to milliseconds

                Returns the elapsed time in the given unit since start() was called or since the creation of the object,
                if start was not called explicitly
            **/
            ROYALE_API int64_t elapsed (const char *type = nullptr) const;    // default is "ms"

            /**
                Returns true is the elapsed time is greater then the given timeout time in milliseconds
                Useful for checking within a loop
            **/
            ROYALE_API bool hasExpired (int64_t timeout) const;

            /**
                Returns the number of seconds for the timepoint where the timer was started / created
            **/
            ROYALE_API int64_t secs() const;

            /**
                Returns the number of ms for the timepoint where the timer was started / created
            **/
            ROYALE_API int64_t msecs() const;

            /**
                Returns the number of us for the timepoint where the timer was started / created
            **/
            ROYALE_API int64_t usecs() const;

            /**
                Returns the number of ns for the timepoint where the timer was started / created
            **/
            ROYALE_API int64_t nsecs() const;

            /**
                Returns the number of milliseconds to reach the other Profiler (time difference for other - this in milliseconds)
            **/
            ROYALE_API int64_t msecsTo (const RoyaleProfiler &other) const;

            /**
                Returns the number of seconds to reach the other Profiler (time difference for other - this in seconds)
            **/
            ROYALE_API int64_t secsTo (const RoyaleProfiler &other) const;

            ROYALE_API bool operator== (const RoyaleProfiler &other) const
            {
                return (m_tpbase == other.m_tpbase) && (m_tpdetail == other.m_tpdetail);
            }

            ROYALE_API bool operator!= (const RoyaleProfiler &other) const
            {
                return ! (*this == other);
            }

            ROYALE_API friend bool operator< (const RoyaleProfiler &v1, const RoyaleProfiler &v2);
            ROYALE_API friend RoyaleProfiler operator+ (const RoyaleProfiler &v1, const RoyaleProfiler &v2);
            ROYALE_API friend RoyaleProfiler operator- (const RoyaleProfiler &v1, const RoyaleProfiler &v2);

        private:
            int64_t m_tpbase;
            int64_t m_tpdetail; // needed on LINUX for gettimeofday
        };

        ROYALE_API bool operator< (const RoyaleProfiler &v1, const RoyaleProfiler &v2);
        ROYALE_API RoyaleProfiler operator+ (const RoyaleProfiler &v1, const RoyaleProfiler &v2);
        ROYALE_API RoyaleProfiler operator- (const RoyaleProfiler &v1, const RoyaleProfiler &v2);
    }
}
