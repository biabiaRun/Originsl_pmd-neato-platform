/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <hal/IPsdTemperatureSensor.hpp>
#include <common/IPseudoDataInterpreter.hpp>
#include <common/NtcTempAlgo.hpp>

#include <memory>

namespace royale
{
    namespace sensors
    {
        /**
        * Pseudo data temperature sensor for NTC temperature sensors
        * to read temperature values from pseudo data.
        */
        class PsdTemperatureSensorNtc : public royale::hal::IPsdTemperatureSensor
        {
        public:
            /**
            * This constructor expects a pseudo data IPseudoDataInterpreter for the pseudo data
            * handling and a NtcTempAlgo to calculate the temperature.
            * \param   psdInterpreter   IPseudoDataInterpreter to get pseudo data.
            * \param   ntcAlgorithm   NTC temperature algorithm to calculate the temperature.
            * \param   phaseSyncConfig   Which phase should be used for the calculation.
            */
            ROYALE_API explicit PsdTemperatureSensorNtc (std::unique_ptr<const royale::common::IPseudoDataInterpreter> psdInterpreter,
                    std::unique_ptr <royale::common::NtcTempAlgo> ntcAlgorithm,
                    royale::hal::IPsdTemperatureSensor::PseudoDataPhaseSync phaseSyncConfig);
            virtual ~PsdTemperatureSensorNtc() override;

            float calcTemperature (const std::vector<common::ICapturedRawFrame *> &frames) override;

        private:
            std::unique_ptr<const royale::common::IPseudoDataInterpreter> m_psdInterpreter;
            std::unique_ptr <royale::common::NtcTempAlgo> m_ntcAlgorithm;
            royale::hal::IPsdTemperatureSensor::PseudoDataPhaseSync m_phaseSyncConfig;
        };
    }
}
