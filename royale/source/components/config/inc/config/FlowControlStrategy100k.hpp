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

#include <common/IFlowControlStrategy.hpp>

namespace royale
{
    namespace common
    {
        /**
         * Flow control strategy for 352 x 287 (100k) px resolution cameras.
         *
         * Depending on exposure times, use:
         * - rawFrameRateSlow for exposureTime greater or equal than 1000us
         * - rawFrameRateFast otherwise
         *
         * The slower rate is given when a long exposure time is already slowing down the data rate.
         * This is not an error; otherwise if there is an exposure time that would exceed the time a
         * raw frame is granted by the raw frame rate limitation, the use case would not be feasible
         * any more. If just one of the raw frames does not fit into the time slot it fails.
         *
         * Note that 100k cams may require USB3 SuperSpeed.
         */
        class FlowControlStrategy100k : public IFlowControlStrategy
        {
        public:
            FlowControlStrategy100k (uint16_t rawFrameRateSlow,
                                     uint16_t rawFrameRateFast);
            uint16_t getRawFrameRate (const royale::usecase::UseCaseDefinition &useCase) override;
        private:
            const uint16_t m_rawFrameRateSlow;
            const uint16_t m_rawFrameRateFast;
        };
    }
}
