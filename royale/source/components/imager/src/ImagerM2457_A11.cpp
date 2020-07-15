/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2457_A11.hpp>
#include <imager/M2457/ImagerRegisters.hpp>
#include <imager/M2457/PseudoDataInterpreter.hpp>
#include <imager/M2457/PseudoDataInterpreter_IC.hpp>

#include <common/exceptions/RuntimeError.hpp>
#include <common/StringFunctions.hpp>
#include <common/MakeUnique.hpp>

using namespace royale::imager;
using namespace royale::common;

ImagerM2457_A11::ImagerM2457_A11 (const ImagerParameters &params) :
    FlashDefinedImagerComponent (params), m_tempSensor(params.tempSensor)
{
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2457_A11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter;
    if (m_tempSensor == config::ImConnectedTemperatureSensor::SIC)
    {
        pseudoDataInter = common::makeUnique<M2457::PseudoDataInterpreter_IC>(m_usesInternalCurrentMonitor);
    }
    else
    {
        pseudoDataInter = common::makeUnique<M2457::PseudoDataInterpreter>(m_usesInternalCurrentMonitor);
    }
    return pseudoDataInter;
}


std::vector < uint16_t > ImagerM2457_A11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (M2457_A11::ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

DesignStepInfo ImagerM2457_A11::getDesignStepInfo()
{
    DesignStepInfo info;
    info.ANAIP_DESIGNSTEP_Address = M2457::ANAIP_DESIGNSTEP;
    info.designSteps.push_back (0x0A11);
    return info;
}

ExpoTimeRegInfo ImagerM2457_A11::getExpoTimeRegInfo()
{
    ExpoTimeRegInfo info;
    info.nSequenceEntries = M2457::nSequenceEntries;
    info.CFGCNT_S00_EXPOTIME_Address = M2457::CFGCNT_S00_EXPOTIME;
    info.CFGCNT_S01_EXPOTIME_Address = M2457::CFGCNT_S01_EXPOTIME;
    info.CFGCNT_FLAGS_Address = M2457::CFGCNT_FLAGS;
    return info;
}

void ImagerM2457_A11::doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData)
{
    uint16_t lowerAddress, upperAddress;

    // Lower the SPI clock to make the transfer more reliable
    const auto spiEnable = 1u << 14;
    const auto spiPolarity = 1u << 12;
    const auto spiClockDiv8 = 2u;
    m_bridge->writeImagerRegister (M2457::SPICFG, static_cast<uint16_t> (spiEnable | spiPolarity | spiClockDiv8));

    upperAddress = static_cast<uint16_t> ( (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0xFF0000) >> 16);
    lowerAddress = static_cast<uint16_t> (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0x00FFFF);

    // Tell the imager where the config can be found
    m_bridge->writeImagerRegister (M2457_A11::USECASE_LOAD_ADDR0, upperAddress);
    m_bridge->writeImagerRegister (M2457_A11::USECASE_LOAD_ADDR1, lowerAddress);

    // Save the current mode to set it again later
    uint16_t prevMode = 0x0000u;
    m_bridge->readImagerRegister (M2457_A11::MODE, prevMode);

    // Set the load usecase mode
    m_bridge->writeImagerRegister (M2457_A11::MODE, 0x0005);

    // Start laoding function by trigering the imager
    m_bridge->writeImagerRegister (M2457_A11::TRIG, 0x0001);
    // Wait until loading is finished
    m_bridge->sleepFor (std::chrono::milliseconds (10));
    m_bridge->writeImagerRegister (M2457_A11::TRIG, 0x0000);

    // Check status bit of usecase loading function
    uint16_t status = 0x0000u;
    m_bridge->readImagerRegister (M2457_A11::STATUS, status);

    if (status & 0x1000)
    {
        throw RuntimeError ("LoadConfigurationFromFlash error : " + royale::common::toStdString (status));
    }

    // Set the old mode again
    m_bridge->writeImagerRegister (M2457_A11::MODE, prevMode);
}
