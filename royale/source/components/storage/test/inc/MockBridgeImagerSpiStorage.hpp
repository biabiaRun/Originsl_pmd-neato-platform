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

#include <hal/IBridgeImager.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

namespace royale
{
    namespace stub
    {
        namespace storage
        {
            /**
             * Simulation of an M2452 with SPI-attached storage.
             *
             * @deprecated it would be better to model the separate hardware parts with separate
             * classes. That method is used for the M2453, by using the Imager component's
             * SimImagerM2453, SimSpiGenericFlash and StubBridgeImager.
             */
            class MockBridgeImagerSpiStorage : public royale::hal::IBridgeImager
            {
            public:
                /**
                 * At the time of writing, only one imager (the M2452) supports acting as an SPI
                 * master, but the StorageSpiImager already takes the imager type as an argument.
                 *
                 * Returns the parameter for StorageSpiImager's constructor.
                 */
                static royale::config::ImagerAsBridgeType getSimulatedImagerType();

                /**
                 * Simulate an imager that can act as an SPI gateway to a flash chip that can store
                 * memorySize bytes.
                 */
                explicit MockBridgeImagerSpiStorage (std::size_t memorySize);
                ~MockBridgeImagerSpiStorage() override;

                /**
                 * Called by the test to set the simulated imager back to unknown state (software imager
                 * state "virgin". The read and write functions will log a test failure if they're called
                 * before calling setImagerReset().
                 */
                void setMustReset();

                /**
                 * Normally the simulation will set the ANAIP_DESIGNSTEP to match the value returned
                 * by getSimulatedImagerType(). This will override it (and the override will be
                 * applied automatically whenever the simulation is reset).
                 */
                void setDesignStep (uint16_t designStep);

                /**
                 * During a read, write or erase operation the imager will fail, and the expected
                 * end condition will not be set (the register that StorageSpiImager's
                 * pollUntilZero is reading will never become zero).
                 *
                 * Returns a small value (between 1k and 2k), the test harness should trigger a read
                 * or write that touches this address.
                 */
                std::size_t setWillBrownoutDuringOperation ();

                void setImagerReset (bool state) override;
                void readImagerRegister (uint16_t regAddr, uint16_t &value) override;
                void writeImagerRegister (uint16_t regAddr, uint16_t value) override;
                void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) override;
                void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) override;
                void sleepFor (std::chrono::microseconds sleepDuration) override;

            private:
                /**
                 * Called after each read and write, simulates the iSM running.
                 *
                 * If the iSM_ENABLE register is zero, this returns immediately.
                 *
                 * If the iSM is enabled, but the firmware hasn't been loaded (checking a few sample
                 * registers are non-zero), this will log a test failure.
                 *
                 * If the iSM is enabled and the firmware has been loaded, it will check which command the
                 * firmware is expected to run, and simulate doing the command immediately (assuming that it's one
                 * of the read/write/erase commands that the test expects). In standalone mode this will set
                 * the iSM_ENABLE register back to zero.
                 */
                void checkRunIsm();

                /**
                 * The registers of the imager itself, will be reset to 64k of all-zero when setImagerReset
                 * (true) is called. This isn't an exact simulation, but the registers specific to the SPI
                 * firmware do have zero as their defaults.
                 */
                std::vector<uint16_t> m_imagerRegisters;

                /**
                 * The flash storage itself, simulated here as a uint16_t-based device because the interface
                 * available via the imager is uint16_t based.
                 */
                std::vector<uint16_t> m_cells;

                bool m_hasBeenReset = false;

                /**
                 * See setWillBrownoutDuringOperation.
                 */
                bool m_willBrownout = false;

                /**
                 * See setDesignStep. The constructor will set this to the normally-expected value.
                 */
                uint16_t m_designStep;
            };
        }
    }
}
