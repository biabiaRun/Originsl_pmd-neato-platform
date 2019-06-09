/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <FrameRateStats.hpp>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace royale::bdd;

void
FrameRateStats::add_timestamp (std::chrono::microseconds ts)
{
    statsValid = false;
    if (!timestamps.empty())
    {
        diffs.push_back (ts - timestamps.back());
    }
    timestamps.push_back (ts);
}


namespace /* anonymous */
{
    // Helper function: Compute mean and standard deviation and count.
    void mean_and_sd (const std::vector<std::chrono::microseconds>::iterator b, const std::vector<std::chrono::microseconds>::iterator e, double &mean, double &sd, size_t &count)
    {
        count = e - b;

        // Initialize...
        mean = NAN;
        sd = NAN;

        if (count < 1)
        {
            return; // insufficient data.
        }

        auto sum = std::accumulate (b, e, std::chrono::microseconds::zero());
        mean = static_cast<double> (sum.count()) / static_cast<double> (count) / 1000000.0;

        if (count < 2)
        {
            return; // insufficient data to compute std deviation.
        }

        // Compute error squares.
        // Cannot use std::accumulate for this due to the data types involved.
        auto sum_err2 = 0.0;
        for (auto it = b; it < e; ++it)
        {
            auto val = static_cast<double> (it->count()) / 1000000.0;
            sum_err2 += (val - mean) * (val - mean);
        }

        sd = sqrt (sum_err2 / (static_cast<double> (count) - 1));
    }
}

void
FrameRateStats::resetStats()
{
    statsValid = false;
    timestamps.clear();
    diffs.clear();
}

void
FrameRateStats::updateStats()
{
    if (statsValid)
    {
        return;
    }

    // Mean and SD over all samples:
    double mean, sd;
    mean_and_sd (diffs.begin(), diffs.end(), mean, sd, avg_fps_n);

    // convert to frame rates:
    avg_fps = 1 / mean;
    avg_fps_sigma = sd / mean / mean;

    // Median:
    std::sort (diffs.begin(), diffs.end());

    median_fps = NAN;
    median_fps_n = diffs.size();
    if (median_fps_n)
    {
        median_fps = 1000000.0 / static_cast<double> (diffs.at (median_fps_n / 2).count());
    }

    // Mean and SD of the middle 90 pct
    auto pct5 = diffs.begin() + diffs.size() / 20;      // 5th percentile
    auto pct95 = diffs.begin() + diffs.size() * 19 / 20; // 95th percentile
    mean_and_sd (pct5, pct95, mean, sd, avg90_fps_n);
    // convert to frame rates:
    avg90_fps = 1 / mean;
    avg90_fps_sigma = sd / mean / mean;

    statsValid = true;
}

