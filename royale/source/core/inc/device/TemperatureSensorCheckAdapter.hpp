/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <hal/ITemperatureSensor.hpp>
#include <common/EventForwarder.hpp>
#include <device/ITemperatureAcceptor.hpp>
#include <memory>

namespace royale
{
    namespace device
    {
        /**
        * Decorator class which wraps an ITemperatureSensor and delegates
        * temperatures to an ITemperatureChecker for temperature checks
        */
        class TemperatureSensorCheckAdapter : public royale::hal::ITemperatureSensor
        {
        public:
            /**
            * This constructor expects an ITemperatureSensor and an ITemperatureChecker for temperature checks
            */
            ROYALE_API TemperatureSensorCheckAdapter (std::shared_ptr<royale::hal::ITemperatureSensor> sensor,
                    std::unique_ptr<ITemperatureAcceptor> checker);

            // implement ITemperatureSensor
            ROYALE_API float getTemperature() override;

        private:
            std::shared_ptr<royale::hal::ITemperatureSensor> m_baseSensor;
            std::unique_ptr<ITemperatureAcceptor> m_checker;

        }; // class TemperatureSensorCheckAdapter

    } // namespace device

} // namespace royale