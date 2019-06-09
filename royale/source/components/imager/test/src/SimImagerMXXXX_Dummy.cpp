/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <SimImagerMXXXX_Dummy.hpp>

#include <common/exceptions/DataNotFound.hpp>

using namespace royale::stub;

SimImagerMXXXX_Dummy::SimImagerMXXXX_Dummy ()
{
    m_SimulatedRegisters = std::map<uint16_t, uint16_t>
    {
    };
}

SimImagerMXXXX_Dummy::~SimImagerMXXXX_Dummy()
{
}

void SimImagerMXXXX_Dummy::writeRegister (uint16_t regAddr, uint16_t value)
{
    // write the value into the simulated register
    m_SimulatedRegisters[regAddr] = value;
}

void SimImagerMXXXX_Dummy::runSimulation (std::chrono::microseconds sleepDuration)
{
    // this class has no asynchronous actions
    (void) sleepDuration;
}

void SimImagerMXXXX_Dummy::startCapturing()
{

}

void SimImagerMXXXX_Dummy::stopCapturing()
{

}

uint16_t SimImagerMXXXX_Dummy::readCurrentRegisterValue (uint16_t regAddr)
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
