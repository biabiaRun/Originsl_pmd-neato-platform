/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2455_A11.hpp>
#include <imager/M2455/ImagerRegisters.hpp>
#include <imager/M2455/PseudoDataInterpreter.hpp>

#include <common/exceptions/RuntimeError.hpp>

#include <cmath>
#include <thread>

using namespace royale::imager;
using namespace royale::common;

ImagerM2455_A11::ImagerM2455_A11 (const ImagerParameters &params) :
    FlashDefinedImagerComponent (params)
{
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2455_A11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new M2455::PseudoDataInterpreter (m_usesInternalCurrentMonitor));
    return pseudoDataInter;
}


std::vector < uint16_t > ImagerM2455_A11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (M2455_A11::ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

DesignStepInfo ImagerM2455_A11::getDesignStepInfo()
{
    DesignStepInfo info;
    info.ANAIP_DESIGNSTEP_Address = M2455::ANAIP_DESIGNSTEP;
    info.designSteps.push_back (0x0A11);
    info.designSteps.push_back (0x0A12);
    info.designSteps.push_back (0x0A13);
    return info;
}

ExpoTimeRegInfo ImagerM2455_A11::getExpoTimeRegInfo()
{
    ExpoTimeRegInfo info;
    info.nSequenceEntries = M2455::nSequenceEntries;
    info.CFGCNT_S00_EXPOTIME_Address = M2455::CFGCNT_S00_EXPOTIME;
    info.CFGCNT_S01_EXPOTIME_Address = M2455::CFGCNT_S01_EXPOTIME;
    info.CFGCNT_FLAGS_Address = M2455::CFGCNT_FLAGS;
    return info;
}

void ImagerM2455_A11::doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData)
{
    uint16_t lowerAddress, upperAddress;

    // Lower the SPI clock to make the transfer more reliable
    const auto spiEnable = 1u << 14;
    const auto spiClockDiv8 = 2u;
    m_bridge->writeImagerRegister (M2455::SPICFG, static_cast<uint16_t> (spiEnable | spiClockDiv8));

    upperAddress = static_cast<uint16_t> ( (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0xFF0000) >> 16);
    lowerAddress = static_cast<uint16_t> (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0x00FFFF);

    // Tell the imager where the config can be found
    m_bridge->writeImagerRegister (M2455_A11::SFR_CFGCNT_GENERIC_FW_PARAM_0, lowerAddress);
    m_bridge->writeImagerRegister (M2455_A11::SFR_CFGCNT_GENERIC_FW_PARAM_1, upperAddress);

    // Enable the correct iSM function
    m_bridge->writeImagerRegister (M2455_A11::FUNCNUM, 0x0007);

    // Set the restart vector
    m_bridge->writeImagerRegister (M2455_A11::RESTARTV_EN, 0x0004);

    // Workaround for a M2455_A11 firmware bug
    m_bridge->writeImagerRegister (0x81CB, 0x0000);

    // Start the iSM function
    m_bridge->writeImagerRegister (M2455_A11::ISM_CTRL, 0x0008);
    m_bridge->sleepFor (std::chrono::milliseconds (10));

    // Read the status of the function
    uint16_t status = 0x0000u;
    m_bridge->readImagerRegister (M2455_A11::SFR_CFGCNT_TESTRES, status);

    if (status == 0xAAAA)
    {
        // The function worked as expected
    }
    else if (status == 0x5555)
    {
        throw RuntimeError ("LoadConfigurationFromFlash error : CRC");
    }
    else if (status == 0x0055)
    {
        throw RuntimeError ("LoadConfigurationFromFlash error : Start address not page aligned");
    }
    else if (status == 0x00AA)
    {
        throw RuntimeError ("LoadConfigurationFromFlash error : Flash is busy");
    }
    else
    {
        LOG (ERROR) << "LoadConfigurationFromFlash error : Unknown status code : " << status;
        throw RuntimeError ("LoadConfigurationFromFlash error : Unknown status code");
    }
}
