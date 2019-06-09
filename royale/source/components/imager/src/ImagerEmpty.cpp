/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerEmpty.hpp>
#include <common/exceptions/LogicError.hpp>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <sstream>

using namespace royale::imager;
using namespace royale::common;

ImagerEmpty::ImagerEmpty (std::shared_ptr<royale::hal::IBridgeImager> bridge,
                          std::unique_ptr<IPseudoDataInterpreter> pdi) :
    ImagerBase (bridge)
{
    if (pdi == nullptr)
    {
        throw LogicError ("nullref exception");
    }

    m_pdi = std::move (pdi);
}

ImagerEmpty::~ImagerEmpty()
{

}

void ImagerEmpty::initialize()
{
}

std::string ImagerEmpty::getSerialNumber()
{
    return "0000-0000-0000-0000";
}

void ImagerEmpty::wake()
{
    //trigger a low pulse on the reset line
    //note: for the Enclustra firmware the release
    //of the signal is ignored, the complete reset
    //sequence is done by the firmware itself when
    //calling the command for the falling edge
    logMessage (ImageSensorLogType::Sleep, "1");
    m_bridge->sleepFor (std::chrono::microseconds (1));
    logMessage (ImageSensorLogType::Reset, "High");
    m_bridge->setImagerReset (false);
}

void ImagerEmpty::sleep()
{
    //set reset line to power down the imager
    logMessage (ImageSensorLogType::Reset, "Low");
    m_bridge->setImagerReset (true);
}

void ImagerEmpty::startCapture()
{

}

void ImagerEmpty::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{

}

void ImagerEmpty::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{

}

void ImagerEmpty::stopCapture()
{
}

ImagerVerificationStatus ImagerEmpty::verifyUseCase (const ImagerUseCaseDefinition &useCase)
{
    return ImagerVerificationStatus::SUCCESS;
}

void ImagerEmpty::executeUseCase (const ImagerUseCaseDefinition &useCase)
{

}

std::vector<std::size_t> ImagerEmpty::getMeasurementBlockSizes() const
{
    return {};
}

void ImagerEmpty::writeRegisters (const std::vector<uint16_t> &registerAddresses,
                                  const std::vector<uint16_t> &registerValues)
{
    writeRegistersInternal (registerAddresses, registerValues);
}

void ImagerEmpty::readRegisters (const std::vector<uint16_t> &registerAddresses,
                                 std::vector<uint16_t> &registerValues)
{
    readRegistersInternal (registerAddresses, registerValues);
}

std::unique_ptr<IPseudoDataInterpreter> ImagerEmpty::createPseudoDataInterpreter()
{
    return std::unique_ptr<IPseudoDataInterpreter> (m_pdi->clone());
}

std::vector < uint16_t > ImagerEmpty::getSerialRegisters()
{
    return {};
}

void ImagerEmpty::setExternalTrigger (bool useExternalTrigger)
{

}
