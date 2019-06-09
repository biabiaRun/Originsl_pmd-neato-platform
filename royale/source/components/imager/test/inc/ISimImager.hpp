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

#include <cstdint>
#include <chrono>

namespace royale
{
    namespace stub
    {
        /**
         * Simulation of the hardware on the other end of the SimulationBridge.
         *
         * When testing the Imager subclasses, the corresponding subclass of ISimImager will
         * provide the expected values for the Imager to pass its sanity checks.  It gives the
         * values expected to avoid the "wrong design step" exception, etc.
         *
         * For some simulations, the actions of the firmware may be simulated immediately before the
         * writeRegister() function returns, others may wait for runSimulation() to be called, or
         * even process asynchronously.
         */
        class ISimImager
        {
        public:
            virtual ~ISimImager() {}
            virtual void writeRegister (uint16_t regAddr, uint16_t value) = 0;
            virtual uint16_t readCurrentRegisterValue (uint16_t regAddr) = 0;
            virtual void startCapturing() = 0; //!< Simulates start of capture (by I2C or an external trigger event)
            virtual void stopCapturing() = 0;  //!< Simulates stop of capture (by I2C or an external trigger event)

            /**
             * Called by StubBridgeImager::sleepFor, allows simulating the firmware running while
             * the software imager sleeps.
             *
             * StubBridgeImager itself does not sleep, if the simulated imager expects sleepDuration
             * of wall-clock time to pass then it must sleep before this function returns.
             */
            virtual void runSimulation (std::chrono::microseconds sleepDuration) = 0;
        };
    }
}
