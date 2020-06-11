/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2455_B12.hpp>
#include <imager/M2455/ImagerRegisters.hpp>

#include <common/exceptions/RuntimeError.hpp>
#include <common/StringFunctions.hpp>

using namespace royale::imager;
using namespace royale::common;

ImagerM2455_B12::ImagerM2455_B12 (const ImagerParameters &params) :
    ImagerM2455_A11 (params)
{
}

DesignStepInfo ImagerM2455_B12::getDesignStepInfo()
{
    DesignStepInfo info;
    info.ANAIP_DESIGNSTEP_Address = M2455::ANAIP_DESIGNSTEP;
    info.designSteps.push_back (0x0B12);
    return info;
}

void ImagerM2455_B12::doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData)
{
    uint16_t lowerAddress, upperAddress;

    // Lower the SPI clock to make the transfer more reliable
    const auto spiEnable = 1u << 14;
    const auto spiPolarity = 1u << 12;
    const auto spiClockDiv8 = 2u;
    m_bridge->writeImagerRegister (M2455::SPICFG, static_cast<uint16_t> (spiEnable | spiPolarity | spiClockDiv8));

    upperAddress = static_cast<uint16_t> ( (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0xFF0000) >> 16);
    lowerAddress = static_cast<uint16_t> (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0x00FFFF);

    // Tell the imager where the config can be found
    m_bridge->writeImagerRegister (M2455_B12::USECASE_LOAD_ADDR0, upperAddress);
    m_bridge->writeImagerRegister (M2455_B12::USECASE_LOAD_ADDR1, lowerAddress);

    // Save the current mode to set it again later
    uint16_t prevMode = 0x0000u;
    m_bridge->readImagerRegister (M2455_B12::MODE, prevMode);

    // Set the load usecase mode
    m_bridge->writeImagerRegister (M2455_B12::MODE, 0x0005);

    // Start laoding function by trigering the imager
    m_bridge->writeImagerRegister (M2455_B12::TRIG, 0x0001);
    // Wait until loading is finished
    m_bridge->sleepFor (std::chrono::milliseconds (10));
    m_bridge->writeImagerRegister (M2455_B12::TRIG, 0x0000);

    // Check status bit of usecase loading function
    uint16_t status = 0x0000u;
    m_bridge->readImagerRegister (M2455_B12::STATUS, status);

    if (status & 0x1000)
    {
        throw RuntimeError ("LoadConfigurationFromFlash error : " + royale::common::toStdString (status));
    }

    // Set the old mode again
    m_bridge->writeImagerRegister (M2455_B12::MODE, prevMode);
}
