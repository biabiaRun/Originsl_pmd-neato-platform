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

namespace royale
{
    namespace device
    {
        /**
        * This interface accepts temperature values of illumination or imager temperature.
        * An implementing class should accept and use the temperature in some way. For example, the
        * implementation can check the temperature against certain limits like, temp <= 150°C.
        */
        class ITemperatureAcceptor
        {
        public:
            virtual ~ITemperatureAcceptor() = default;

            /**
            * receives the given temperature
            * \param  temp temperature to accept in centigrade
            */
            virtual void acceptTemperature (float temp) const = 0;

        }; // class ITemperatureChecker

    } // namespace device

} // namespace royale
