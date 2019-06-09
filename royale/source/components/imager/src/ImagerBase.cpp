/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <imager/ImagerBase.hpp>
#include <algorithm>
#include <common/NarrowCast.hpp>
#include <common/StringFunctions.hpp>
#include <iomanip>
#include <thread>
#include <set>
#include <iomanip>

using namespace royale::imager;
using namespace royale::common;

/**
* Any event that is logged before the first successful readout of the serial number
* registers will have this identifier assigned to identify the imager object it was
* sent from. When the concrete imager implementation assigns the real serial number
* to m_serial in its implementation of IImager::getSerialNumber it should display
* this identifier one last time.
*/
uint32_t ImagerBase::s_imagerIdCounter = 0;

ImagerBase::ImagerBase (const std::shared_ptr<royale::hal::IBridgeImager> &bridge) :
    m_loggingListener (nullptr)
{
    if (bridge == nullptr)
    {
        throw LogicError ("nullref exception");
    }

    m_bridge = bridge;

    m_imagerMyId = s_imagerIdCounter;
    s_imagerIdCounter++;
    m_serial = "Unidentified device " + toStdString (m_imagerMyId);
}

void ImagerBase::setLoggingListener (IImageSensorLoggingListener *listener)
{
    m_loggingListener = listener;
}

void ImagerBase::logMessage (ImageSensorLogType logType, const std::string &logMessage)
{
    if (m_loggingListener != nullptr)
    {
        m_loggingListener->onImagerLogEvent (m_serial, std::chrono::high_resolution_clock::now(), logType, logMessage);
    }
}

void ImagerBase::logRegister (ImageSensorLogType logType, uint16_t address, uint16_t value)
{
    std::stringstream logMsg;
    logMsg << "0x" << std::setfill ('0') << std::setw (4) << std::hex << address;
    logMsg << ";";
    logMsg << "0x" << std::setfill ('0') << std::setw (4) << std::hex << value;
    logMessage (logType, logMsg.str());
}

void ImagerBase::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    m_bridge->readImagerRegister (regAddr, value);
    logRegister (ImageSensorLogType::I2CRead, regAddr, value);
}

void ImagerBase::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    logRegister (ImageSensorLogType::I2CWrite, regAddr, value);
    m_bridge->writeImagerRegister (regAddr, value);
}

void ImagerBase::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    logMessage (ImageSensorLogType::BurstStart, toStdString (values.size()));
    const auto nValues = values.size();
    for (size_t i = 0; i < nValues; i++)
    {
        logRegister (ImageSensorLogType::I2CWrite, static_cast<uint16_t> (firstRegAddr + i), values.at (i));
    }
    m_bridge->writeImagerBurst (firstRegAddr, values);
    logMessage (ImageSensorLogType::BurstEnd, toStdString (values.size()));
}

void ImagerBase::writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                 const std::vector<uint16_t> &registerValues)
{
    //autoconf imagers don't support this
    throw NotImplemented();
}

void ImagerBase::readRegisters (const std::vector<uint16_t> &registerAddresses,
                                std::vector<uint16_t> &registerValues)
{
    //autoconf imagers don't support this
    throw NotImplemented();
}

void ImagerBase::writeRegistersInternal (const std::vector<uint16_t> &registerAddresses,
        const std::vector<uint16_t> &registerValues)
{
    if (registerAddresses.size() != registerValues.size())
    {
        throw LogicError ("vector length mismatch of arguments");
    }

    size_t index = 0;
    uint16_t lastAddress = 0;
    bool continuous = true;
    for (const auto regAdr : registerAddresses)
    {
        if (index)
        {
            if (regAdr - lastAddress != 1)
            {
                continuous = false;
            }
        }

        lastAddress = regAdr;
        index++;
    }

    if (continuous && registerAddresses.size() > 1)
    {
        m_bridge->writeImagerBurst (registerAddresses[0], registerValues);
    }
    else
    {
        for (auto i = 0u; i < registerValues.size(); ++i)
        {
            m_bridge->writeImagerRegister (registerAddresses[i], registerValues[i]);
        }
    }
}

void ImagerBase::readRegistersInternal (const std::vector<uint16_t> &registerAddresses,
                                        std::vector<uint16_t> &registerValues)
{
    if (registerAddresses.size() != registerValues.size())
    {
        throw LogicError ("vector length mismatch of arguments");
    }

    bool continuous = true;
    uint16_t lastAddress = 0;
    size_t index = 0;
    for (const auto regAdr : registerAddresses)
    {
        if (index)
        {
            if (regAdr - lastAddress != 1)
            {
                continuous = false;
            }
        }

        lastAddress = regAdr;
        index++;
    }

    if (continuous && registerAddresses.size())
    {
        m_bridge->readImagerBurst (registerAddresses[0], registerValues);
    }
    else
    {
        for (auto i = 0u; i < registerValues.size(); ++i)
        {
            m_bridge->readImagerRegister (registerAddresses[i], registerValues[i]);
        }
    }
}

void ImagerBase::trackAndWriteRegisters (const std::map < uint16_t, uint16_t > &registers)
{
    if (registers.empty())
    {
        return;
    }

    //download to imager
    uint16_t firstAddress = registers.begin()->first;
    uint16_t currentAddress = 0;
    std::vector<uint16_t> regBatch{};

    auto writeAndRemember = [ = ] (uint16_t firstAddress, std::vector<uint16_t> &regBatch)
    {
        logMessage (ImageSensorLogType::BurstStart, toStdString (regBatch.size()));

        //a consecutive batch is ready to be written
        m_bridge->writeImagerBurst (firstAddress, regBatch);

        //write succeeded if no exception interrupted the call
        for (auto regBlockAddress : regBatch)
        {
            logRegister (ImageSensorLogType::I2CWrite, firstAddress, regBlockAddress);
            LOG (INFO) << "Register written: 0x" << std::hex << firstAddress << " <= 0x" << std::hex << regBlockAddress;

            //remember what was written
            m_regDownloaded[firstAddress++] = regBlockAddress;
        }

        logMessage (ImageSensorLogType::BurstEnd, toStdString (regBatch.size()));
    };

    for (auto reg : registers)
    {
        //check for consecutivity
        if (currentAddress != reg.first - 1)
        {
            if (regBatch.size())
            {
                writeAndRemember (firstAddress, regBatch);

                //clear batch and proceed with next one
                regBatch.clear();
                firstAddress = reg.first;
            }

            currentAddress = reg.first;
        }
        else
        {
            currentAddress++;
        }

        //it was a consecutive address entry, stage for burst write
        regBatch.push_back (reg.second);
    }

    if (regBatch.size())
    {
        writeAndRemember (firstAddress, regBatch);
    }
}

void ImagerBase::trackShadowedRegisters (const std::map < uint16_t, uint16_t > &registers)
{
    if (! m_regPendingShadow.empty())
    {
        throw LogicError ("Multiple shadowed-register operations are in flight");
    }
    m_regPendingShadow = registers;
    for (const auto reg : registers)
    {
        m_regDownloaded.erase (reg.first);
    }
}

void ImagerBase::commitOrRollbackShadowedRegisters (bool success)
{
    if (success)
    {
        for (const auto reg : m_regPendingShadow)
        {
            m_regDownloaded[reg.first] = reg.second;
        }
    }
    m_regPendingShadow.clear();
}

void ImagerBase::maskedWriteRegister (uint16_t address, uint16_t mask, uint16_t value, uint16_t resetValue)
{
    uint16_t oldValue = resetValue;

    if (m_regDownloaded.count (address))
    {
        oldValue = m_regDownloaded[address];
    }

    const uint16_t newValue = static_cast<uint16_t> ( (oldValue & ~mask) | (value & mask));
    trackAndWriteRegisters ({ { address, newValue } });
}

std::map < uint16_t, uint16_t > ImagerBase::resolveConfiguration (const std::map < uint16_t, uint16_t > &regChanges)
{
    //create map for gathering all differences
    std::map < uint16_t, uint16_t > regDifferences;

    for (auto reg : regChanges)
    {
        //if the change never has been downloaded before or
        // the previously downloaded value differs
        if (m_regDownloaded.count (std::get<0> (reg)) == 0 ||
                m_regDownloaded[std::get<0> (reg)] != std::get<1> (reg))
        {
            regDifferences[std::get<0> (reg)] = std::get<1> (reg);
        }
    }

    return regDifferences;
}
