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

#include <hal/IPsdTemperatureSensor.hpp>
#include <common/EventForwarder.hpp>
#include <device/ITemperatureAcceptor.hpp>
#include <memory>

namespace royale
{
    namespace device
    {
        /**
        * Decorator class which wraps an IPsdTemperatureSensor and delegates
        * temperatures to an ITemperatureChecker for temperature checks
        */
        class PsdTemperatureSensorCheckAdapter : public royale::hal::IPsdTemperatureSensor
        {
        public:
            /**
            * This constructor expects an IPsdTemperatureSensor and an ITemperatureChecker for temperature checks
            */
            ROYALE_API PsdTemperatureSensorCheckAdapter (std::shared_ptr<royale::hal::IPsdTemperatureSensor> sensor,
                    std::shared_ptr<ITemperatureAcceptor> checker);

            // implement IPsdTemperatureSensor
            ROYALE_API float calcTemperature (const std::vector<common::ICapturedRawFrame *> &frames) override;

        private:
            std::shared_ptr<royale::hal::IPsdTemperatureSensor> m_baseSensor;
            std::shared_ptr<ITemperatureAcceptor> m_checker;

        }; // class PsdTemperatureSensorCheckAdapter

    } // namespace device

} // namespace royale
