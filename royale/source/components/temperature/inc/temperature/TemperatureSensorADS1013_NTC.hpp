/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <common/NtcTempAlgo.hpp>
#include <pal/II2cDeviceAccess.hpp>
#include <hal/ITemperatureSensor.hpp>

#include <memory>

namespace royale
{
    namespace sensors
    {
        /**
        * This class can be used for NTC temperature sensors that are connected
        * to a ADS1013 ADC (http://www.ti.com/product/ADS1013).
        */
        class TemperatureSensorADS1013_NTC : public royale::hal::ITemperatureSensor
        {
        public:
            ROYALE_API explicit TemperatureSensorADS1013_NTC (std::shared_ptr<pal::II2cDeviceAccess> device, std::unique_ptr <royale::common::NtcTempAlgo> ntcAlgorithm);
            ~TemperatureSensorADS1013_NTC() override;
            float getTemperature() override;

        private:
            bool m_initialized;
            std::shared_ptr<pal::II2cDeviceAccess> m_device;
            std::unique_ptr <royale::common::NtcTempAlgo> m_ntcAlgorithm;
        };
    }
}
