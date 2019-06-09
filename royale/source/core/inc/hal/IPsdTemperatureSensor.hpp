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

#include <royale/Definitions.hpp>
#include <common/ICapturedRawFrame.hpp>
#include <vector>

namespace royale
{
    namespace hal
    {
        /**
        * Interface for temperature sensors read out via the pseudo data from the imager.
        * An example of such a sensor is the PsdTemperatureSensorNtc.
        */
        class IPsdTemperatureSensor
        {
        public:

            /**
            * Defines which phase should be used for the NTC measurement.
            * This depends on the NTC firmware implementation that you are using :
            *
            * ImagerM2452_B1x_AIO : For this implementation you should choose the first
            *                       phase without illumination
            * ImagerM2453_A11 : For this implementation the measurement values in the very
            *                   first phase are uninitialized because the temperature is
            *                   measured during the read out. So you should choose the
            *                   second phase of every sequence.
            *
            * The value you are choosing applies to the whole ModuleConfig which means
            * that you have to ensure that it is valid for every use case.
            */
            enum class PseudoDataPhaseSync
            {
                NOT_SYNCHRONIZED,
                FIRST,
                SECOND,
                LAST
            };

            virtual ~IPsdTemperatureSensor() = default;

            /**
            * Calculates a temperature value out of the pseudo data from the given frames
            * filters it if appropriate and returns it in centigrade.
            * \param   frames   The frames that contain the pseudo data to process.
            */
            virtual float calcTemperature (const std::vector<common::ICapturedRawFrame *> &frames) = 0;
        };
    }
}
