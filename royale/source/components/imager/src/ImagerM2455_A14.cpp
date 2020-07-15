/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2455_A14.hpp>
#include <imager/M2455/ImagerRegisters.hpp>

using namespace royale::imager;
using namespace royale::common;

ImagerM2455_A14::ImagerM2455_A14 (const ImagerParameters &params) :
    ImagerM2455_A11 (params)
{
}

DesignStepInfo ImagerM2455_A14::getDesignStepInfo()
{
    DesignStepInfo info;
    info.ANAIP_DESIGNSTEP_Address = M2455::ANAIP_DESIGNSTEP;
    info.designSteps.push_back (0x0A14);
    return info;
}
