/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/M2452/ImagerRegisters.hpp>
#include <utility>

namespace platform
{
    const std::map < uint16_t, uint16_t > BaseConfig
    {
        //************************************************************
        // Assumptions:
        // - Start trigger is done by I2C --> GPIO0 is set to high Z
        // - CSI2 --> 1 Lane (A11 only) and Super Frame enabled
        // - Sunny HW with 1W VCSEL --> Eye Safety realized in HW
        //************************************************************
        { royale::imager::M2452::CFGCNT_PSOUT, 0x0311 },         // ==> equals default value enable Duty Cycle control for MOD_SE_PAD
        { royale::imager::M2452::ANAIP_PADGPIOCFG0, 0x1313 },    // set also GPIO0 to highZ
        { royale::imager::M2452::ANAIP_PSPADCFG, 0x1513 },       // enable MOD_SE_P
        { royale::imager::M2452::CFGCNT_CSICFG, 0x0192 },        // CSI Configuration 2 Lane , Pseudo data ON, Super Frame ON

        { royale::imager::M2452::ANAIP_DPHYDLANE1CFG1, 0x01C5 }, //DPHY DLANE 1 configuration register 1
        { royale::imager::M2452::ANAIP_DPHYDLANE2CFG1, 0x01C5 }, //DPHY DLANE 2 configuration register 1
        { royale::imager::M2452::ANAIP_DPHYDLANE1CFG3, 0x0206 }, //DPHY DLANE 1 configuration register 3
        { royale::imager::M2452::ANAIP_DPHYDLANE2CFG3, 0x0206 }  //DPHY DLANE 2 configuration register 3
    };
}
