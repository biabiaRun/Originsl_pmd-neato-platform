/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2453_A11.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2453/PseudoDataInterpreter.hpp>

using namespace royale::imager;
using namespace royale::common;

ImagerM2453_A11::ImagerM2453_A11 (const ImagerParameters &params) :
    ImagerM2453 (params)
{
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2453_A11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new M2453_A11::PseudoDataInterpreter (m_usesInternalCurrentMonitor));
    return pseudoDataInter;
}

std::vector < uint16_t > ImagerM2453_A11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (M2453_A11::ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

DesignStepInfo ImagerM2453_A11::getDesignStepInfo()
{
    DesignStepInfo info;
    info.ANAIP_DESIGNSTEP_Address = M2453::ANAIP_DESIGNSTEP;
    info.designSteps.push_back (0x0A11);
    info.designSteps.push_back (0x0A12);
    return info;
}
