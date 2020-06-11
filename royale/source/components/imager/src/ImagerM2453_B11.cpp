/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2453_B11.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2453/PseudoDataInterpreter_B11.hpp>

#include <common/exceptions/RuntimeError.hpp>

using namespace royale::imager;
using namespace royale::common;


ImagerM2453_B11::ImagerM2453_B11 (const ImagerParameters &params) :
    ImagerM2453 (params)

{
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2453_B11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new M2453_B11::PseudoDataInterpreterB11 (m_usesInternalCurrentMonitor));
    return pseudoDataInter;
}

std::vector < uint16_t > ImagerM2453_B11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (M2453_B11::ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

DesignStepInfo ImagerM2453_B11::getDesignStepInfo()
{
    DesignStepInfo info;
    info.ANAIP_DESIGNSTEP_Address = M2453::ANAIP_DESIGNSTEP;
    info.designSteps.push_back (0x0B11);
    info.designSteps.push_back (0x0B12);
    return info;
}

void ImagerM2453_B11::doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData)
{
    uint16_t lowerAddress, upperAddress;

    // Lower the SPI clock to make the transfer more reliable
    const auto spiEnable = 1u << 14;
    const auto spiClockDiv8 = 2u;
    m_bridge->writeImagerRegister (M2453::SPICFG, static_cast<uint16_t> (spiEnable | spiClockDiv8));

    upperAddress = static_cast<uint16_t> ( (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0xFF0000) >> 16);
    lowerAddress = static_cast<uint16_t> (useCaseData.sequentialRegisterHeader.flashConfigAddress & 0x00FFFF);

    // Tell the imager where the config can be found
    m_bridge->writeImagerRegister (M2453_B11::SFR_CFGCNT_GENERIC_FW_PARAM_0, lowerAddress);
    m_bridge->writeImagerRegister (M2453_B11::SFR_CFGCNT_GENERIC_FW_PARAM_1, upperAddress);

    // Enable the correct iSM function
    m_bridge->writeImagerRegister (M2453_B11::FUNCNUM, 0x0007);

    // Set the restart vector
    m_bridge->writeImagerRegister (M2453_B11::RESTARTV_EN, 0x0004);

    // Start the iSM function
    m_bridge->writeImagerRegister (M2453_B11::ISM_CTRL, 0x0008);
    m_bridge->sleepFor (std::chrono::milliseconds (10));

    // Read the status of the function
    uint16_t status = 0x0000u;
    m_bridge->readImagerRegister (M2453_B11::SFR_CFGCNT_TESTRES, status);

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
