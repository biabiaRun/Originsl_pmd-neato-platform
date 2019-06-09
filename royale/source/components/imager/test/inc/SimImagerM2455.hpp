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

#include <ISimImager.hpp>

#include <SimSpiGenericFlash.hpp>

#include <map>
#include <memory>
#include <vector>

namespace royale
{
    namespace stub
    {
        /**
         * The main sensor that's capturing the raw data, with an attached SPI storage.
         *
         * This is a very simplified model of the imager for unit testing only.
         * Especially it does not cover:
         * - Any timings
         * - Blocking of I2C communication while SPI communication is ongoing
         */
        class SimImagerM2455 : public ISimImager
        {
        public:
            explicit SimImagerM2455 ();
            ~SimImagerM2455() override;
            void writeRegister (uint16_t regAddr, uint16_t value) override;
            uint16_t readCurrentRegisterValue (uint16_t regAddr) override;
            void startCapturing() override;
            void stopCapturing() override;
            void runSimulation (std::chrono::microseconds sleepDuration) override;

            const std::map<uint16_t, uint16_t> getImagerRegisters() const
            {
                return m_simulatorRegisters;
            }

            /**
             * The contents of the simulated storage may be read and modified by the caller, using
             * the returned pointer. However, the caller must ensure that the map is not accessed
             * at the same time as runSimulation() is accessing the map.
             */
            std::shared_ptr<std::map<uint32_t, uint8_t>> getFlashMemorySpace()
            {
                return m_flashMemory.getFlashMemorySpace();
            }

        private:
            std::map<uint16_t, uint16_t> m_simulatorRegisters{};
            SimSpiGenericFlash m_flashMemory;

            void initiateSpiTransfer();
        };
    }
}

