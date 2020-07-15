/****************************************************************************\
 * Copyright (C) 2020 Infineon Technologies
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

#include <memory>

namespace royale
{
    namespace sensors
    {
        /**
        * Pseudo data temperature sensor for NTC temperature sensors
        * to read temperature values from pseudo data.
        */
        class PsdTemperatureSensorIc : public royale::hal::IPsdTemperatureSensor
        {
        public:
            /**
            * This constructor expects a pseudo data IPseudoDataInterpreter for the pseudo data
            * handling.
            * \param   psdInterpreter   IPseudoDataInterpreter to get pseudo data.
            */
            ROYALE_API explicit PsdTemperatureSensorIc (std::unique_ptr<const royale::common::IPseudoDataInterpreter> psdInterpreter);
            ROYALE_API virtual ~PsdTemperatureSensorIc() override;

            ROYALE_API float calcTemperature (const std::vector<common::ICapturedRawFrame *> &frames) override;

        private:
            std::unique_ptr<const royale::common::IPseudoDataInterpreter> m_psdInterpreter;
        };
    }
}
