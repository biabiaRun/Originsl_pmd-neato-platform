/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerMXXXX_Dummy.hpp>
#include <imager/M2452/PseudoDataInterpreter_AIO.hpp>

#include <common/exceptions/NotImplemented.hpp>

#include <thread>

using namespace royale::imager;
using namespace royale::common;


ImagerMXXXX_Dummy::ImagerMXXXX_Dummy (const ImagerParameters &params) :
    m_bridge{ params.bridge },
    m_externalConfig{ params.externalConfig }
{
    if (m_bridge == nullptr)
    {
        throw LogicError ("nullref exception");
    }

    if (!m_externalConfig)
    {
        throw RuntimeError ("this imager needs to have an external configuration provided");
    }

    if (!m_externalConfig->getUseCaseList().size())
    {
        throw RuntimeError ("the external configuration needs to contain at least one use case");
    }
}

ImagerMXXXX_Dummy::~ImagerMXXXX_Dummy()
{

}

void ImagerMXXXX_Dummy::initialize()
{
    for (const auto &r : m_externalConfig->getInitializationMap())
    {
        m_bridge->writeImagerRegister (r.address, r.value);
    }
}

std::string ImagerMXXXX_Dummy::getSerialNumber()
{
    return "0000-0000-0000-0000";
}

void ImagerMXXXX_Dummy::wake()
{
    std::this_thread::sleep_for (std::chrono::microseconds (1));
    m_bridge->setImagerReset (false);
}

void ImagerMXXXX_Dummy::sleep()
{
    //set reset line to power down the imager
    m_bridge->setImagerReset (true);
}

void ImagerMXXXX_Dummy::startCapture()
{
    for (const auto &r : m_externalConfig->getStartMap())
    {
        m_bridge->writeImagerRegister (r.address, r.value);
    }
}

void ImagerMXXXX_Dummy::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{
    throw NotImplemented();
}

void ImagerMXXXX_Dummy::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    throw NotImplemented();
}

uint16_t ImagerMXXXX_Dummy::stopCapture()
{
    for (const auto &r : m_externalConfig->getStopMap())
    {
        m_bridge->writeImagerRegister (r.address, r.value);
    }
    return 0u;
}

ImagerVerificationStatus ImagerMXXXX_Dummy::verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier)
{
    return ImagerVerificationStatus::SUCCESS;
}

void ImagerMXXXX_Dummy::executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier)
{
    //this is a dummy imager that accepts (and ignores) any use case passed to
    //this method - it always load the first use case defined in the external configuration

    for (const auto &r : m_externalConfig->getUseCaseList().at (0).registerMap)
    {
        m_bridge->writeImagerRegister (r.address, r.value);
    }
}

std::vector<std::size_t> ImagerMXXXX_Dummy::getMeasurementBlockSizes() const
{
    return{ m_externalConfig->getUseCaseList().at (0).imageStreamBlockSizes };
}

void ImagerMXXXX_Dummy::setLoggingListener (IImageSensorLoggingListener *pListener)
{
    throw NotImplemented();
}

std::unique_ptr<IPseudoDataInterpreter> ImagerMXXXX_Dummy::createPseudoDataInterpreter()
{
    // ROYAL-2734 says that this imager should use M2452 data until \todo ROYAL-2712 has been resolved
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new royale::imager::M2452::PseudoDataInterpreter_AIO);
    return pseudoDataInter;
}

void ImagerMXXXX_Dummy::setExternalTrigger (bool useExternalTrigger)
{
    throw NotImplemented();
}
