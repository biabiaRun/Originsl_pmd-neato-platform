/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <SimImagerM2450_A12_AIO.hpp>
#include <imager/M2450_A12/ImagerRegisters.hpp>
#include <common/exceptions/DataNotFound.hpp>

using namespace royale::imager::M2450_A12;
using namespace royale::stub;

namespace
{
    const uint16_t AIO_SR_RECONFIGFLAGS = CFGCNT_S30_PS;
    const uint16_t AIO_SR_RECONFIG_COUNTER = CFGCNT_S30_PLLSET;
    const uint16_t AIO_SR_TRIGGER = CFGCNT_S31_EXPOTIME;
}

SimImagerM2450_A12_AIO::SimImagerM2450_A12_AIO()
{
    m_SimulatedRegisters = std::map<uint16_t, uint16_t>
    {
        { ANAIP_DESIGNSTEP, 0x0A12 },
        { ANAIP_EFUSEVAL1, 0x0100 },
        { CFGCNT_STATUS, m_statusIdle },
        { MTCU_STATUS, 0x0001 },
        { iSM_ISMSTATE, 0x1000 },
        { MTCU_SEQNUM, 0x0000 },
        { CFGCNT_TRIG, 0x0000 },
        { 0xB04F, 0x5529 },
        { 0xB050, 0x5555 },
        { 0xB051, 0x4E15 },
        { 0xB052, 0xC71B },
        { 0xB053, 0x0000 },
        { RECONFIG_COUNTER, 0x0000 },
        { AIO_SR_RECONFIGFLAGS, 0x0000 },
        { AIO_SR_RECONFIG_COUNTER, 0x0000 }
    };

    //the firmware will set this value after its first run; the simulated imager
    //does not make any assumptions about the initial state (the test must write
    //the CFGCNT_STATUS register by itself), but this class will assume that
    //the firmware has been started before any start/stop caputure call and will
    //use the value below to indicate the idle state after a stopCapturing call
    m_statusIdle = 0x8001;
}

void SimImagerM2450_A12_AIO::writeRegister (uint16_t regAddr, uint16_t value)
{
    // write the value into the simulated register
    m_SimulatedRegisters[regAddr] = value;

    // simulate the imager reacting to the register value change
    switch (regAddr)
    {
        case iSM_EN:
            if (m_SimulatedRegisters[iSM_CTRL] == 1)
            {
                if (1 == value)
                {
                    // reset bit 2 of MTCU_STATUS
                    m_SimulatedRegisters[MTCU_STATUS] &= static_cast<uint16_t> (~ (0x1 << 2));
                    m_SimulatedRegisters[iSM_ISMSTATE] = 0x1000;
                }
            }

            if (m_SimulatedRegisters[iSM_CTRL] == 2)
            {
                if (2 == value)
                {
                    m_SimulatedRegisters[iSM_ISMSTATE] = 0u;
                }
            }
            break;
        case CFGCNT_TRIG:
            if ( (value & 0x01) == 1)
            {
                startCapturing();
            }
            else
            {
                stopCapturing();
            }
            break;

        case AIO_SR_RECONFIGFLAGS:
            //the firmware would clear this flag on successful copy from the safe-reconfig
            //to the shadow-config space, the simulation always succeeds here and clears the flag
            m_SimulatedRegisters[AIO_SR_RECONFIGFLAGS] = 0;
            break;

        case AIO_SR_TRIGGER:
            {
                if ( (value & 0x01) == 1)
                {
                    m_SimulatedRegisters[iSM_ISMSTATE] = (1 << 14);
                    startCapturing();
                }

                if ( (value & 0x01) == 0)
                {
                    stopCapturing();
                }
                break;
            }
        default:
            break;
    }
}

void SimImagerM2450_A12_AIO::runSimulation (std::chrono::microseconds sleepDuration)
{
    // this class has no asynchronous actions
    (void) sleepDuration;
}

void SimImagerM2450_A12_AIO::startCapturing()
{
    // busy, no errors
    m_SimulatedRegisters[CFGCNT_STATUS] = 0;
    m_SimulatedRegisters[MTCU_STATUS] = 0;
}

void SimImagerM2450_A12_AIO::stopCapturing()
{
    // idle, no errors
    m_SimulatedRegisters[CFGCNT_STATUS] = m_statusIdle;
    m_SimulatedRegisters[MTCU_STATUS] = 1;
}

uint16_t SimImagerM2450_A12_AIO::readCurrentRegisterValue (uint16_t regAddr)
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
