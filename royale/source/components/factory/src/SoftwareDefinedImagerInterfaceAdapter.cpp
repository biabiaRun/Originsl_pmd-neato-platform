/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/SoftwareDefinedImagerInterfaceAdapter.hpp>
#include <factory/ImagerUseCaseDefinitionAdapter.hpp>
#include <common/NarrowCast.hpp>

using namespace royale::factory;

SoftwareDefinedImagerInterfaceAdapter::
SoftwareDefinedImagerInterfaceAdapter (std::shared_ptr<royale::imager::ISoftwareDefinedImagerComponent> imager) :
    m_imager (std::move (imager))
{
}

std::shared_ptr<royale::hal::IImager> SoftwareDefinedImagerInterfaceAdapter::
createImager (std::shared_ptr<royale::imager::ISoftwareDefinedImagerComponent> imager)
{
    return std::make_shared<SoftwareDefinedImagerInterfaceAdapter> (std::move (imager));
}

void SoftwareDefinedImagerInterfaceAdapter::wake()
{
    m_imager->wake();
}

std::unique_ptr<royale::common::IPseudoDataInterpreter> SoftwareDefinedImagerInterfaceAdapter::createPseudoDataInterpreter()
{
    return m_imager->createPseudoDataInterpreter();
}

std::string SoftwareDefinedImagerInterfaceAdapter::getSerialNumber()
{
    return m_imager->getSerialNumber();
}

void SoftwareDefinedImagerInterfaceAdapter::initialize()
{
    m_imager->initialize();
}

void SoftwareDefinedImagerInterfaceAdapter::sleep()
{
    m_imager->sleep();
}

royale::usecase::VerificationStatus SoftwareDefinedImagerInterfaceAdapter::
verifyUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate)
{
    switch (m_imager->verifyUseCase (factory::ImagerUseCaseDefinitionAdapter (useCase, roiCMin, roiRMin, flowControlRate)))
    {
        case imager::ImagerVerificationStatus::DUTYCYCLE:
            return royale::usecase::VerificationStatus::DUTYCYCLE;
            break;
        case imager::ImagerVerificationStatus::EXPOSURE_TIME:
            return royale::usecase::VerificationStatus::EXPOSURE_TIME;
            break;
        case imager::ImagerVerificationStatus::FRAMERATE:
            return royale::usecase::VerificationStatus::FRAMERATE;
            break;
        case imager::ImagerVerificationStatus::MODULATION_FREQUENCY:
            return royale::usecase::VerificationStatus::MODULATION_FREQUENCY;
            break;
        case imager::ImagerVerificationStatus::PHASE:
            return royale::usecase::VerificationStatus::PHASE;
            break;
        case imager::ImagerVerificationStatus::REGION:
            return royale::usecase::VerificationStatus::REGION;
            break;
        case imager::ImagerVerificationStatus::SUCCESS:
            return royale::usecase::VerificationStatus::SUCCESS;
            break;
        default:
            return royale::usecase::VerificationStatus::UNDEFINED;
    }

    return royale::usecase::VerificationStatus::UNDEFINED;
}

void SoftwareDefinedImagerInterfaceAdapter::
executeUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate)
{
    m_imager->executeUseCase (factory::ImagerUseCaseDefinitionAdapter (useCase, roiCMin, roiRMin, flowControlRate));
}

void SoftwareDefinedImagerInterfaceAdapter::startCapture()
{
    m_imager->startCapture();
}

void SoftwareDefinedImagerInterfaceAdapter::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{
    m_imager->reconfigureExposureTimes (exposureTimes, reconfigIndex);
}

void SoftwareDefinedImagerInterfaceAdapter::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    m_imager->reconfigureTargetFrameRate (targetFrameRate, reconfigIndex);
}

void SoftwareDefinedImagerInterfaceAdapter::stopCapture()
{
    m_imager->stopCapture();
}

royale::Vector<std::size_t> SoftwareDefinedImagerInterfaceAdapter::getMeasurementBlockSizes() const
{
    return m_imager->getMeasurementBlockSizes();
}

void SoftwareDefinedImagerInterfaceAdapter::setExternalTrigger (bool useExternalTrigger)
{
    m_imager->setExternalTrigger (useExternalTrigger);
}

void SoftwareDefinedImagerInterfaceAdapter::writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers)
{
    std::vector<uint16_t> registerAddresses;
    std::vector<uint16_t> registerValues;

    for (const auto reg : registers)
    {
        uint64_t regAdr64 = 0;

        if (0 == reg.first.find ("0x"))
        {
            std::stringstream ss (reg.first.c_str());
            ss >> std::hex >> regAdr64;
        }
        else
        {
            std::stringstream ss (reg.first.c_str());
            ss >> std::dec >> regAdr64;
        }
        auto regAdr = common::narrow_cast<uint16_t> (regAdr64);

        registerAddresses.push_back (regAdr);
        registerValues.push_back (common::narrow_cast<uint16_t> (reg.second));
    }

    m_imager->writeRegisters (registerAddresses, registerValues);
}

void SoftwareDefinedImagerInterfaceAdapter::readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers)
{
    std::vector<uint16_t> registerAddresses;
    std::vector<uint16_t> registerValues;

    for (const auto reg : registers)
    {
        uint64_t regAdr64 = 0;

        if (0 == reg.first.find ("0x"))
        {
            std::stringstream ss (reg.first.c_str());
            ss >> std::hex >> regAdr64;
        }
        else
        {
            std::stringstream ss (reg.first.c_str());
            ss >> std::dec >> regAdr64;
        }
        auto regAdr = common::narrow_cast<uint16_t> (regAdr64);

        registerAddresses.push_back (regAdr);
    }

    registerValues.resize (registerAddresses.size(), 0u);
    m_imager->readRegisters (registerAddresses, registerValues);

    for (size_t idx = 0; idx < registers.size(); idx++)
    {
        registers.at (idx).second = registerValues.at (idx);
    }
}
