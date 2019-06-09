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
#include <vector>

namespace royale
{
    namespace hal
    {
        /**
         * Interface of Bridge functions for controlling the main data-capture source.
         *
         * This controls the sensor hardware, but does not provide a channel to receive the image
         * data.  The data channel is IBridgeDataReceiver, and an implementation may choose to use a
         * single class to implement both interfaces.
         *
         * The Imager is an I2C device with 16-bit addresses and 16-bit values.
         */
        class IBridgeImager
        {
        public:
            virtual ~IBridgeImager() = default;

            /**
             * Lowers or raises the imager's reset I/O pin.
             *
             * \param state false for normal operation, true to acivate reset state.
             */
            virtual void setImagerReset (bool state) = 0;

            virtual void readImagerRegister (uint16_t regAddr, uint16_t &value) = 0;
            virtual void writeImagerRegister (uint16_t regAddr, uint16_t value) = 0;

            /**
             * Reads the values of a sequential block of registers. This call may be implemented as
             * multiple reads, with any locking and thread safety applying only to the individual
             * reads.
             *
             * @param firstRegAddr address that the read starts from
             * @param values the size of this vector defines how many registers are read, the vector is
             * populated with the data read from the imager.
             */
            virtual void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) = 0;

            /**
             * Writes the values of a sequential block of registers. This call may be implemented as
             * multiple writes, with any locking and thread safety applying only to the individual
             * writes.  Unless the imager has been stopped before this call, a new UseCaseDefinition
             * capture could start with a mix of old and new register values.
             *
             * @param firstRegAddr address that the read starts from
             * @param values the size of this vector defines how many registers are read, the vector is
             * populated with the data read from the imager.
             */
            virtual void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) = 0;

            /**
             * Bridge depending sleep function.
             * In the case of normal operation or hardware simulations, the sleep is for the given number of
             * microseconds, as experienced by the device. For unit tests, the simulated device might have
             * time running faster than wall-clock time to speed up the unit test. For hardware simulations,
             * the sleep will be for the time it takes the simulation to run as real silicon would do in the
             * requested time.
             *
             * @param sleepDuration duration of sleep in microseconds
             */
            virtual void sleepFor (std::chrono::microseconds sleepDuration) = 0;
        };
    }
}
