/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <vector>
#include <chrono>

namespace royale
{
    namespace bdd
    {
        class FrameRateStats
        {
        public:
            void add_timestamp (std::chrono::microseconds ts);
            void resetStats();
            void updateStats();

            /*
             * Frame rates are calculated from the differences of consecutive timestamps.
             * This is more precise than just waiting for a fixed amount of time and counting
             * the frames.
             *
             * Regarding the various averages:
             * - avg_fps/avg_fps_sigma
             *   Arithmetic mean of the frame rate, taking all samples into account.
             *   Frame drops may cause this value to be lower than expected.
             * - acg90_fps/avg90_fps_sigma
             *   Same as above, but the shortest and longest 5% (each) of samples
             *   aren't counted. This should reduce the influence of frame drops.
             * - median_fps
             *   The median frame rate (i.e. the middle value of samples sorted by frame rate).
             *   This represents a "typical" sample. In case of irregularily spaced exposures
             *   (e.g. for mixed mode usecases), may significantly differ from the mean frame
             *   rates above.
             *
             * The "avg_fps" value should be close to that obtained by the simple method of
             * counting frames for a given time; it may be slightly higher due to systematic
             * error in the simple method. In contrast to the simple method it also gives the
             * standard deviation, which may be useful to determine jitter and frame drops.
             *
             * The "avg90_fps" value should give a good estimate of the frame rate, discounting
             * frame drops (if they aren't excessive). It should be a good match to the advertised
             * frame rate of the use case. For mixed mode, it should also be usable (per stream, at
             * least), but a high standard deviation is to be expected if the use case has irregularily
             * spaced exposures.
             *
             * Finally, the "median_fps" should give a "typical" frame rate. For irregularily spaced
             * exposures (e.g. mixed mode), this may differ significantly from the mean values above;
             * in the regular case it's expected to match these values.
             *
             * Regarding the standard deviation:
             * As the frame rates aren't expected to have a normal distribution, at least some of
             * the well-known properties of the standard deviation (e.g. 68.2% of samples are within one
             * sigma) don't hold.
             *
             */
            bool   statsValid;
            double avg_fps;         // arithmetic mean
            double avg_fps_sigma;   // standard deviation of the above
            size_t avg_fps_n;       // n samples used for calculating the above
            double avg90_fps;       // arithmetic mean of 5th-95th percentile
            double avg90_fps_sigma; // standard deviation of the above
            size_t avg90_fps_n;     // n samples used for calculating the above (about 90% of avg_fps_n)
            double median_fps;      // median
            size_t median_fps_n;    // n samples used in the median (same as avg_fps_n).

        private:
            std::vector<std::chrono::microseconds> timestamps;
            std::vector<std::chrono::microseconds> diffs;

        }; // class StreamData

    }
}
