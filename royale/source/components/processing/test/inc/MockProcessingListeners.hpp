/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/IDepthDataListener.hpp>

#include <ThreadedAssertSupport.hpp>

namespace royale
{
    namespace stub
    {
        namespace processing
        {
            /**
             * A minimal listener that does no checking.
             */
            class MockDepthDataListener : public royale::IDepthDataListener, public royaletest::ThreadedAssertSupport
            {
            public:
                void onNewData (const royale::DepthData *data) override;
            };

            /**
             * This listener checks that callbacks are received in the right sequence,
             * for example that the timestamps are monotonically increasing.
             */
            class SequenceCheckingDepthDataListener : public royale::IDepthDataListener, public royaletest::ThreadedAssertSupport
            {
            public:
                void onNewData (const royale::DepthData *data) override;

            private:
                std::map<royale::StreamId, std::chrono::microseconds> m_lastTimestamp;
            };
        }
    }
}
