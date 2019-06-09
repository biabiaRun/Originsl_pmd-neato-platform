/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <SimImagerM2452.hpp>
#include <chrono>

using namespace royale::imager::M2452;
using namespace royale::stub;

namespace
{
    const auto AIO_SR_RECONFIGFLAGS = CFGCNT_S15_PS;
    const auto AIO_SR_RECONFIG_COUNTER = CFGCNT_S16_EXPOTIME;
}

SimImagerM2452::SimImagerM2452 (uint16_t designStep)
{
    m_SimulatedRegisters = std::map<uint16_t, uint16_t>
    {
        { ANAIP_DESIGNSTEP, designStep },
        { ANAIP_EFUSEVAL1, 0x0100 },
        { ANAIP_EFUSEVAL2, 0x0000 },
        { ANAIP_EFUSEVAL3, 0x0000 },
        { ANAIP_EFUSEVAL4, 0x0000 },
        { MTCU_STATUS, 0x0003 },
        { ISM_ISMSTATE, 0x1000 },
        { RECONFIG_COUNTER, 0x0000 },
        { RECONFIG_TRIGGER, 0x0000 },
        { AIO_SR_RECONFIGFLAGS, 0x0000 },
        { AIO_SR_RECONFIG_COUNTER, 0x0000 }
    };

    m_doShutdown = false;
    m_simRunning = true;
    m_hwTimingThread = std::thread (&SimImagerM2452::hwTiming, this);
    m_processedMtcuLpfsmem = 0;
}

SimImagerM2452::~SimImagerM2452()
{
    {
        std::unique_lock<std::mutex> lk (m_hwTimingLock);
        m_simRunning = false;
        m_simShutDown.notify_all();
    }
    m_hwTimingThread.join();
}

void SimImagerM2452::hwTiming()
{
    while (m_simRunning)
    {
        std::unique_lock<std::mutex> lk (m_hwTimingLock);
        m_simShutDown.wait_for (lk, std::chrono::milliseconds (1));

        //wait allows spurious wake-up therefore check m_doShutdown too
        if (m_simRunning && m_doShutdown)
        {
            stopCapturing();
            m_doShutdown = false;
        }
    }
}

void SimImagerM2452::writeRegister (uint16_t regAddr, uint16_t value)
{
    // write the value into the simulated register
    m_SimulatedRegisters[regAddr] = value;
}

void SimImagerM2452::runSimulation (std::chrono::microseconds sleepDuration)
{
    //the firmware would clear this flag on successful copy from the safe-reconfig
    //to the shadow-config space, the simulation always succeeds here and clears the flag
    m_SimulatedRegisters[AIO_SR_RECONFIGFLAGS] = 0;

    if (m_SimulatedRegisters.count (MTCU_LPFSMEN))
    {
        auto value = m_SimulatedRegisters[MTCU_LPFSMEN];
        if (value == m_processedMtcuLpfsmem)
        {
            // do nothing, it hasn't changed since the previous runSimulation
        }
        else if ( (value & 0x04) != 0) //Starts LPFSM sequence
        {
            startCapturing();
        }
        else
        {
            //stop only if running (no system change if already stopped or not yet started)
            if (0u == m_SimulatedRegisters[MTCU_STATUS])
            {
                //trigger delayed shutdown (simulation of sequencer)
                m_doShutdown = true;
                m_simShutDown.notify_all();
            }
        }
    }

    std::this_thread::sleep_for (sleepDuration);
}

void SimImagerM2452::startCapturing()
{
    // busy, no errors
    m_SimulatedRegisters[MTCU_STATUS] = 0;
}

void SimImagerM2452::stopCapturing()
{
    //set lpfsm_ready and mtcu_ready to idle
    m_SimulatedRegisters[MTCU_STATUS] = 3;
}

uint16_t SimImagerM2452::readCurrentRegisterValue (uint16_t regAddr)
{
    if (m_SimulatedRegisters.find (regAddr) == m_SimulatedRegisters.end())
    {
        throw royale::common::DataNotFound ("This register's standard value is not defined");
    }
    else
    {
        return m_SimulatedRegisters.at (regAddr);
    }
}
