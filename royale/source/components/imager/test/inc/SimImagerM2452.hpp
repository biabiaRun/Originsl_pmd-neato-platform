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

#include <ISimImager.hpp>
#include <imager/M2452/ImagerRegisters.hpp>
#include <common/exceptions/DataNotFound.hpp>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <map>
#include <mutex>
#include <chrono>

namespace royale
{
    namespace stub
    {
        /**
         * The main sensor that's capturing the raw data.
         */
        class SimImagerM2452 : public ISimImager
        {
        public:
            explicit SimImagerM2452 (uint16_t designStep);
            ~SimImagerM2452() override;
            void writeRegister (uint16_t regAddr, uint16_t value) override;
            uint16_t readCurrentRegisterValue (uint16_t regAddr) override;
            void startCapturing() override;
            void stopCapturing() override;
            void runSimulation (std::chrono::microseconds sleepDuration) override;

        private:
            std::map<uint16_t, uint16_t> m_SimulatedRegisters;
            std::thread m_hwTimingThread;
            std::mutex m_hwTimingLock;
            std::condition_variable m_simShutDown;

            std::atomic<bool> m_simRunning;
            std::atomic<bool> m_doShutdown;
            void hwTiming();

            // The value of MTCU_LPFSMEM when runSimulation() last ran, for checking whether it has
            // changed.
            uint16_t m_processedMtcuLpfsmem;
        };
    }
}
