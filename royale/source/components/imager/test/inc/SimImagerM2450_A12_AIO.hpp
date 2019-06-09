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

#include <ISimImager.hpp>

#include <map>

namespace royale
{
    namespace stub
    {
        class SimImagerM2450_A12_AIO : public ISimImager
        {
        public:
            SimImagerM2450_A12_AIO();
            void writeRegister (uint16_t regAddr, uint16_t value) override;
            uint16_t readCurrentRegisterValue (uint16_t regAddr) override;
            void startCapturing() override;
            void stopCapturing() override;
            void runSimulation (std::chrono::microseconds sleepDuration) override;

        private:
            uint16_t m_statusIdle;
            std::map<uint16_t, uint16_t> m_SimulatedRegisters;
        };
    }
}
