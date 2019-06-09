/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <mutex>
#include <string>

namespace royaletest
{
    class ThreadedAssertSupport
    {
    public:
        ROYALE_API ~ThreadedAssertSupport() = default;
        /**
         * Does a GTest assert if a previous call to assertEq failed.  This is to transfer the
         * failure on to the main test thread.
         *
         * If multiple calls to assertEq failed, it returns the first one.
         */
        ROYALE_API void checkForThreadedAssert();

    protected:
        /**
         * For threaded testing, if the assert fails then this throws a royale Exception (in the
         * current thread) and also sets checkForThreadedAssert to do a GTest assert when called.
         */
        template<typename T>
        void assertEq (const T &a, const T &b, const std::string &reason)
        {
            if (a != b)
            {
                fail (reason);
            }
        }

        /**
         * For threaded testing, if the assert fails then this throws a royale Exception (in the
         * current thread) and also sets checkForThreadedAssert to do a GTest assert when called.
         */
        template<typename T>
        void assertTrue (const T &a, const std::string &reason)
        {
            if (!a)
            {
                fail (reason);
            }
        }

    private:
        /**
         * If non-empty, a previous call to one of the assert functions failed.
         */
        std::string m_assertReason;
        std::mutex m_mutex;
        ROYALE_API void fail (const std::string &reason);
    };
}
