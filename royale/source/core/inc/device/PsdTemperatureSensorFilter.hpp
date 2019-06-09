/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <hal/IPsdTemperatureSensor.hpp>
#include <common/WeightedAverage.hpp>
#include <memory>

namespace royale
{
    namespace device
    {
        /**
        * Decorator class which wraps an IPsdTemperatureSensor and filters
        * temperatures via an WeightedAverage algorithm
        */
        class PsdTemperatureSensorFilter : public royale::hal::IPsdTemperatureSensor
        {
        public:
            /**
            * This constructor expects an IPsdTemperatureSensor temperature
            * filtering and a weight value for the weight of the
            * temperature values.
            */
            ROYALE_API PsdTemperatureSensorFilter (std::shared_ptr<royale::hal::IPsdTemperatureSensor> sensor, float weight = 0.03f);

            // implement IPsdTemperatureSensor
            ROYALE_API float calcTemperature (const std::vector<common::ICapturedRawFrame *> &frames) override;

        private:
            std::shared_ptr<royale::hal::IPsdTemperatureSensor> m_baseSensor; //!< the real temperature sensor
            royale::common::WeightedAverage m_average; //!< the weighted average algorithm.

        }; // class PsdTemperatureSensorFilter

    } // namespace device

} // namespace royale
