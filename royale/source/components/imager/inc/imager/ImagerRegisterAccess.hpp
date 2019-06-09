/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <hal/IBridgeImager.hpp>
#include <config/IImagerExternalConfig.hpp>
#include <imager/ImagerCommon.hpp>

#include <chrono>
#include <vector>

namespace royale
{
    namespace imager
    {
        /**
        * Wrapper class around IBridgeImager providing handling for accessing multiple registers.
        * The writes to consecutive registers are bundled in to a single I/O operation, and timings
        * in TimedRegisterMaps are handled.
        */
        class ImagerRegisterAccess
        {
        public:
            IMAGER_EXPORT explicit ImagerRegisterAccess (const std::shared_ptr<royale::hal::IBridgeImager> &bridge);
            IMAGER_EXPORT ~ImagerRegisterAccess() = default;

            /**
            * Write values to a (possibly non-contiguous) set of addresses.
            *
            * If all of the addresses are sorted and consecutive, the I/O will be optimized in to a
            * single I/O operation. Otherwise, the decision of whether to bundle consecutive writes
            * is implementation-defined.
            */
            IMAGER_EXPORT void writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                               const std::vector<uint16_t> &registerValues);
            /**
            * Read values from a (possibly non-contiguous) set of addresses.
            *
            * If all of the addresses are sorted and consecutive, the I/O will be optimized in to a
            * single I/O operation. Otherwise, the decision of whether to bundle consecutive reads
            * is implementation-defined.
            */
            IMAGER_EXPORT void readRegisters (const std::vector<uint16_t> &registerAddresses,
                                              std::vector<uint16_t> &registerValues);

            /**
            * The content of the TimedRegisterMap is transferred to the hardware imager.
            * For this the IBridgeImager function writeImagerBurst is used.
            * If registers are declared in consecutive order they will be transferred
            * as one block, using a single call to writeImagerBurst.
            * Except if a register address/value pair has a delay value set.
            * Then a block is written, the delay is executed and afterwards it will
            * be continued with writing the next block.
            *
            * \param  registerMap       The register map, containing address/value pairs
            *                           as well as optional sleep times.
            */
            IMAGER_EXPORT void transferRegisterMapAuto (const TimedRegisterList &registerMap);

            /**
            * This will unconditionally sleep for firstSleep, and then in a loop read register reg,
            * if it's not equal to expectedVal it will sleep for pollSleep and then retry (repeatedly).
            *
            * \param   reg           The register address of the register which's value should be polled for
            * \param   expectedVal   The register value which is expected to end the polling
            * \param   firstSleep    Time to sleep before the polling is started
            * \param   pollSleep     Time to sleep between every poll
            * \throw Timeout after a number of retries (the number is defined in the function)
            */
            IMAGER_EXPORT void pollUntil (uint16_t reg,
                                          const uint16_t expectedVal,
                                          const std::chrono::microseconds firstSleep,
                                          const std::chrono::microseconds pollSleep);

        private:
            std::shared_ptr<royale::hal::IBridgeImager> m_bridge;
        };
    }
}
