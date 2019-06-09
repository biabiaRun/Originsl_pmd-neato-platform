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
         * A flow control strategy that returns a constant raw frame rate.
         */
        class FlowControlStrategyFixed : public IFlowControlStrategy
        {
        public:
            explicit FlowControlStrategyFixed (uint16_t rawFrameRate);
            uint16_t getRawFrameRate (const royale::usecase::UseCaseDefinition &useCase) override;
        private:
            const uint16_t m_rawFrameRate;
        };
    }
}
