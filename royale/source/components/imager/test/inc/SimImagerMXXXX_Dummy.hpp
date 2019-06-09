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
        /**
         * The main sensor that's capturing the raw data.
         */
        class SimImagerMXXXX_Dummy : public ISimImager
        {
        public:
            SimImagerMXXXX_Dummy();
            ~SimImagerMXXXX_Dummy();
            void writeRegister (uint16_t regAddr, uint16_t value) override;
            uint16_t readCurrentRegisterValue (uint16_t regAddr) override;
            void startCapturing() override;
            void stopCapturing() override;
            void runSimulation (std::chrono::microseconds sleepDuration) override;

        private:
            std::map<uint16_t, uint16_t> m_SimulatedRegisters;
        };
    }
}
