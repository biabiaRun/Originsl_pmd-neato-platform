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

#include <royale/Definitions.hpp>
#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * Information about accessing hardware that is connected via the Bridge.
         *
         * How this information is interpreted depends on the Bridge, and the Bridge could interpret the same ISensorRoutingConfig differently for different roles.
         *
         * For example:
         * * The Bridge has a module connected via USB.
         * * Inside the module are two I2C busses, bus 1 has the MAIN_IMAGER and bus 2 has all other sensors.
         * * Inside the module is a USB to I2C bridge chip, so part of the Bridge's role is to drive this chip.
         *
         * With this example, and depending on how why this type of module has the devices split across busses this way, either of the following would be possible:
         * * The choice of bus is based on the SensorRole, and it's hardcoded in the Bridge that the MAIN_IMAGER is on a separate bus.  This would use ISensorRoutingConfig subclass SensorRoutingConfigI2c.
         * * The choice of bus is included in the ISensorRoutingConfig (using a new subclass).
         *
         * Also, the Imager is treated specially, sending commands via IBridgeImager instead of
         * IBridgeWithI2c.  The Imager may have an I2C based routing, even if the Bridge doesn't
         * support other sensors using I2C communication.
         */
        class ISensorRoutingConfig
        {
        public:
            virtual ~ISensorRoutingConfig() {};
        };
    }
}
