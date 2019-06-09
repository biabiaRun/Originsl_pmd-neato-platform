/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <StubBridgeImager.hpp>
#include <common/exceptions/LogicError.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>

using namespace royale::common;
using namespace royale::stub;
using namespace std::chrono;

StubBridgeImager::StubBridgeImager (std::shared_ptr <ISimImager> simImager, std::ostream &file) :
    m_file (file),
    m_simImager (std::move (simImager)),
    m_enableCorruptedCommunication (false),
    m_registerCalls (0)
{
    m_file << "";
    if (nullptr == m_simImager)
    {
        throw LogicError ("StubBridgeImager simImager can not be null");
    }
}

StubBridgeImager::~StubBridgeImager()
{

}

ISimImager &StubBridgeImager::getImager()
{
    return *m_simImager.get();
}

void StubBridgeImager::setImagerReset (bool state)
{
    if (state)
    {
        m_hasBeenReset = true;
        clearRegisters();
    }
}

void StubBridgeImager::setCorruptedCommunication ()
{
    m_enableCorruptedCommunication = true;
}

void StubBridgeImager::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    std::lock_guard<std::mutex> lock (m_rwLock);

    if (!m_hasBeenReset)
    {
        throw LogicError("Imager has not been reset");
    }

    if (m_enableCorruptedCommunication)
    {
        m_enableCorruptedCommunication = false;
        throw Exception();
    }

    value = m_simImager->readCurrentRegisterValue (regAddr);

    // you can use the next line to get the register dump on the console
    // default is disabled because otherwise the UTs are dumping too much data
    // m_file << "read  (val<adr):0x" << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << value << "<0x" << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << regAddr << "\n";

    m_registerCalls++;
}

void StubBridgeImager::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    std::lock_guard<std::mutex> lock (m_rwLock);

    if (!m_hasBeenReset)
    {
        throw LogicError("Imager has not been reset");
    }

    if (m_enableCorruptedCommunication)
    {
        m_enableCorruptedCommunication = false;
        throw Exception();
    }

    //add the register to the written ones
    m_regWritten[regAddr] = value;

    m_simImager->writeRegister (regAddr, value);

    // you can use the next line to get the register dump on the console
    // default is disabled because otherwise the UTs are dumping too much data
    // m_file << "write (val>adr):0x" << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << value << ">0x" << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << regAddr << "\n";

    m_registerCalls++;
}

void StubBridgeImager::readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values)
{
    if (!m_hasBeenReset)
    {
        throw LogicError("Imager has not been reset");
    }

    auto registerCalls = m_registerCalls;
    for (uint32_t i = 0; i < values.size(); i++)
    {
        readImagerRegister (static_cast<uint16_t> (firstRegAddr + i), values[i]);
    }

    m_registerCalls = registerCalls + 1;
}

void StubBridgeImager::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    if (!m_hasBeenReset)
    {
        throw LogicError("Imager has not been reset");
    }

    auto registerCalls = m_registerCalls;
    for (uint32_t i = 0; i < values.size(); i++)
    {
        writeImagerRegister (static_cast<uint16_t> (firstRegAddr + i), values[i]);
    }

    m_registerCalls = registerCalls + 1;
}

void StubBridgeImager::clearRegisters()
{
    m_regWritten.clear();
}

std::map < uint16_t, uint16_t > StubBridgeImager::getWrittenRegisters()
{
    return m_regWritten;
}

void StubBridgeImager::resetRegisterCalls()
{
    m_registerCalls = 0;
}

uint32_t StubBridgeImager::registerCalls()
{
    return m_registerCalls;
}

void StubBridgeImager::sleepFor (std::chrono::microseconds sleepDuration)
{
    m_simImager->runSimulation (sleepDuration);
}
