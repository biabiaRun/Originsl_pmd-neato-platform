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
#include <royale/IEvent.hpp>

namespace royale
{
    namespace event
    {

        /**
        * Statistics for raw frames and frame drops.
        */
        class EventRawFrameStats : public royale::IEvent
        {
        public:
            ROYALE_API EventRawFrameStats (uint16_t totalFrames, uint16_t frameDropsBridge, uint16_t frameDropsCollector);

            // implement IEvent
            royale::EventSeverity severity() const override;
            const royale::String describe() const override;
            royale::EventType type() const override;

            // data
            uint16_t m_totalFrames;
            uint16_t m_frameDropsBridge;
            uint16_t m_frameDropsCollector;
        };
    }
}
