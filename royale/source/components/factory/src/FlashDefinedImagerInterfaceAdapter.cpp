/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/FlashDefinedImagerInterfaceAdapter.hpp>
#include <factory/ImagerUseCaseDefinitionAdapter.hpp>
#include <common/NarrowCast.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/NotImplemented.hpp>

using namespace royale::factory;

FlashDefinedImagerInterfaceAdapter::
FlashDefinedImagerInterfaceAdapter (std::shared_ptr<royale::imager::IFlashDefinedImagerComponent> imager, std::shared_ptr<royale::imager::ImagerRegisterAccess> directAccess) :
    m_imager (std::move (imager)),
    m_directAccess (std::move (directAccess))
{
}

std::shared_ptr<royale::hal::IImager> FlashDefinedImagerInterfaceAdapter::
createImager (std::shared_ptr<royale::imager::IFlashDefinedImagerComponent> imager, std::shared_ptr<royale::imager::ImagerRegisterAccess> directAccess)
{
    return std::make_shared<FlashDefinedImagerInterfaceAdapter> (std::move (imager), std::move (directAccess));
}

void FlashDefinedImagerInterfaceAdapter::wake()
{
    m_imager->wake();
}

std::unique_ptr<royale::common::IPseudoDataInterpreter> FlashDefinedImagerInterfaceAdapter::createPseudoDataInterpreter()
{
    return m_imager->createPseudoDataInterpreter();
}

std::string FlashDefinedImagerInterfaceAdapter::getSerialNumber()
{
    return m_imager->getSerialNumber();
}

void FlashDefinedImagerInterfaceAdapter::initialize()
{
    m_imager->initialize();
}

void FlashDefinedImagerInterfaceAdapter::sleep()
{
    m_imager->sleep();
}

royale::usecase::VerificationStatus FlashDefinedImagerInterfaceAdapter::
verifyUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate)
{
    switch (m_imager->verifyUseCase (royale::imager::toImagerUseCaseIdentifier (useCase.getIdentifier())))
    {
        case imager::ImagerVerificationStatus::SUCCESS:
            return royale::usecase::VerificationStatus::SUCCESS;
            break;
        default:
            return royale::usecase::VerificationStatus::UNDEFINED;
    }

    return royale::usecase::VerificationStatus::UNDEFINED;
}

void FlashDefinedImagerInterfaceAdapter::
executeUseCase (const royale::usecase::UseCaseDefinition &useCase, uint16_t roiCMin, uint16_t roiRMin, uint16_t flowControlRate)
{
    m_imager->executeUseCase (royale::imager::toImagerUseCaseIdentifier (useCase.getIdentifier()));
    m_executingUcd = useCase;
}

void FlashDefinedImagerInterfaceAdapter::startCapture()
{
    m_imager->startCapture();
}

void FlashDefinedImagerInterfaceAdapter::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{
    // We're getting exposure times per raw frame set as input,
    // but the imager expects exposure times per sequence entry.
    const auto &rfs = m_executingUcd.getRawFrameSets();

    if (rfs.size() != exposureTimes.size())
    {
        throw common::InvalidValue ("reconfigureExposureTimes: unexpected amount of exposure times");
    }

    std::vector<uint32_t> exposuresPerSequence;
    auto rfs_it = rfs.cbegin();
    for (auto exp : exposureTimes)
    {
        const auto nrawframes = rfs_it->countRawFrames();
        exposuresPerSequence.insert(exposuresPerSequence.end(), nrawframes, exp);
        ++rfs_it;
    }

    m_imager->reconfigureExposureTimes (exposuresPerSequence, reconfigIndex);
}

void FlashDefinedImagerInterfaceAdapter::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    m_imager->reconfigureTargetFrameRate (targetFrameRate, reconfigIndex);
}

void FlashDefinedImagerInterfaceAdapter::stopCapture()
{
    m_imager->stopCapture();
}

royale::Vector<std::size_t> FlashDefinedImagerInterfaceAdapter::getMeasurementBlockSizes() const
{
    return m_imager->getMeasurementBlockSizes();
}

void FlashDefinedImagerInterfaceAdapter::setExternalTrigger (bool useExternalTrigger)
{
    m_imager->setExternalTrigger (useExternalTrigger);
}

void FlashDefinedImagerInterfaceAdapter::writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers)
{
    if (!m_directAccess)
    {
        throw common::NotImplemented();
    }

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

    m_directAccess->writeRegisters (registerAddresses, registerValues);
}

void FlashDefinedImagerInterfaceAdapter::readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers)
{
    if (!m_directAccess)
    {
        throw common::NotImplemented();
    }

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
    m_directAccess->readRegisters (registerAddresses, registerValues);

    for (size_t idx = 0; idx < registers.size(); idx++)
    {
        registers.at (idx).second = registerValues.at (idx);
    }
}
