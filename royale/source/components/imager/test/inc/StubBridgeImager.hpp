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

#include <hal/IBridgeImager.hpp>
#include <ISimImager.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <iostream>

namespace royale
{
    namespace stub
    {
        /**
         * A virtual IBridgeImager with simulated imager hardware to respond to reading and writing
         * registers.
         */
        class StubBridgeImager : public royale::hal::IBridgeImager
        {
        public:
            StubBridgeImager (std::shared_ptr <royale::stub::ISimImager> simImager, std::ostream &file = std::cout);

            ~StubBridgeImager();

            void setImagerReset (bool state) override;

            void readImagerRegister (uint16_t regAddr, uint16_t &value) override;

            void writeImagerRegister (uint16_t regAddr, uint16_t value) override;

            void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) override;

            void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) override;

            void sleepFor (std::chrono::microseconds sleepDuration) override;

            /**
             * Clears all entries from the written registers map.
             */
            void clearRegisters();

            /*
             * Lets the next call to read or write a register throw an exception
             */
            void setCorruptedCommunication ();

            /**
             * Getter method for the written registers map.
             */
            std::map < uint16_t, uint16_t > getWrittenRegisters();

            ISimImager &getImager();

            void resetRegisterCalls();
            uint32_t registerCalls();

        private:
            std::ostream &m_file;
            std::shared_ptr <royale::stub::ISimImager> m_simImager;
            std::map < uint16_t, uint16_t > m_regWritten;
            std::mutex m_rwLock;

            bool m_enableCorruptedCommunication;
            bool m_hasBeenReset = false;

            uint32_t m_registerCalls;
        };
    }
}
