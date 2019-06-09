#include <iostream>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include <common/RoyaleProfiler.hpp>

using namespace royale::common;

// delivers uncertain information and therefore cannot be used for UTs
TEST (TestRoyaleProfiler, DISABLED_testTimer)
{
    RoyaleProfiler profiler;
    std::this_thread::sleep_for (std::chrono::milliseconds (10));
    uint64_t mSecsvalue = profiler.msecsElapsed();

    ASSERT_NEAR ( (double) 10, (double) mSecsvalue, 2);
}


TEST (TestRoyaleProfiler, DISABLED_testWait)
{
    RoyaleProfiler profiler, profile_total;

    while (!profiler.hasExpired (10));    // as 10 ms have not expired, continue the loop
    ASSERT_NEAR ( (double) 10, (double) profiler.msecsElapsed(), 2);
}

TEST (TestRoyaleProfiler, DISABLED_testDifference)
{
    RoyaleProfiler profiler;

    std::this_thread::sleep_for (std::chrono::milliseconds (20));

    RoyaleProfiler profile_total;

    ASSERT_LT (profiler, profile_total);
    ASSERT_NEAR ( (double) 20, (double) (profile_total - profiler).msecs(), 5);
}

// delivers uncertain information and therefore cannot be used for UTs
TEST (TestRoyaleProfiler, DISABLED_testReset)
{
    RoyaleProfiler profiler;

    std::this_thread::sleep_for (std::chrono::microseconds (200000));

    ASSERT_NEAR ( (double) 200000, (double) profiler.usecsElapsed(), 5000);
    profiler.reset();
    ASSERT_NEAR ( (double) 0, (double) profiler.usecsElapsed(), 2);
}