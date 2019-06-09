/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#pragma once

#include <cstdint>
#include <cmath>

namespace royale
{
    namespace common
    {
        class NtcTempAlgo
        {
        public:
            NtcTempAlgo (float resistorR1, float resistorRntc0, float referenceTemperature, float thermistorBeta);

            float calcTemperature (uint16_t vRef1, uint16_t vNtc1, uint16_t vRef2, uint16_t vNtc2, uint16_t offset) const;
            float calcTemperature(float refVoltage, float ntcVoltage) const;

        private:
            float m_resistorRatio;        //!< ratio between R1 and NTC at reference temperature
            float m_reciRefTp;            //!< reciprocal reference temperature
            float m_reciTBeta;            //!< reciprocal thermistor beta
            static const float celsiusKelvinFactor;  //!< difference between temperature in Kelvin and degree Celsius

            float calcResistorVoltageRatio (float refVoltage, float ntcVoltage) const;
            void checkDifferences (float refVoltage, float ntcVoltage) const;
        };

    }
}
