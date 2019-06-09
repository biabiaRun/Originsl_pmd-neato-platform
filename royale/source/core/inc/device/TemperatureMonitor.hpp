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

#include <common/EventForwarder.hpp>
#include <device/ITemperatureAcceptor.hpp>
#include <memory>

namespace royale
{
    namespace device
    {
        /**
        * Class which monitors temperatures and generates
        * events if temperature limits are exceeded.
        */
        class TemperatureMonitor : public ITemperatureAcceptor
        {
        public:
            ROYALE_API explicit TemperatureMonitor (float softLimit, float hardLimit, float hysteresis = 1.0f);


            ROYALE_API void setEventListener (royale::IEventListener *listener);

            ROYALE_API bool softAlarm() const;
            ROYALE_API bool hardAlarm() const;

            ROYALE_API void retrigger();

            // implement ITemperatureAcceptor
            ROYALE_API virtual void acceptTemperature (float temp) const override;

        private:
            royale::EventForwarder m_eventForwarder;
            float m_softLimit;
            float m_hardLimit;
            float m_hysteresis;
            mutable bool m_softLimitTriggered;
            mutable bool m_hardLimitTriggered;

        }; // class TemperatureMonitor

    } // namespace device

} // namespace royale