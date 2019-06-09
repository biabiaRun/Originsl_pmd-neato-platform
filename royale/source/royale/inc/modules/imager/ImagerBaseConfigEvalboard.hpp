/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/M2450_A12/ImagerRegisters.hpp>
#include <utility>

namespace royale
{
    namespace imager
    {
        namespace M2450_A12
        {
            using namespace royale::imager::M2450_A12;

            /** \brief  This is the default register configuration for an Infineon EvalBoard.
            *  It should be possible to change use-case specific registers based on this configuration.
            *  This configuration is created for 26MHz XOSC frequency and configures the imager to a 133MHz system clock.
            *  Check project-royale\\doc\\imager_configurations a more readable description of the MiraCE evalboard default configuration. */

            const std::map < uint16_t, uint16_t > BaseConfigForEvalboard
            {
                { MTCU_POWERCTRL, 0x13FF },   //MTCU_POWERCTRL
                { ANAIP_GPIOMUX5, 0x0180 },   // ANAIP_GPIOMUX5     // for Eval Kit Illumination Board Control
                { ANAIP_GPIOMUX6, 0x3140 },   // ANAIP_GPIOMUX6     // for Eval Kit Illumination Board Control
                { ANAIP_GPIOMUX7, 0x018D },   // ANAIP_GPIOMUX7     // for Eval Kit Illumination Board Control
                { ANAIP_PADGPIOCFG0, 0x1515 },   // ANAIP_PADGPIOCFG0
                { ANAIP_PADGPIOCFG1, 0x1515 },   // ANAIP_PADGPIOCFG1
                { ANAIP_PADGPIOCFG2, 0x1515 },   // ANAIP_PADGPIOCFG2
                { ANAIP_PADGPIOCFG3, 0x1515 },   // ANAIP_PADGPIOCFG3
                { ANAIP_PADGPIOCFG4, 0x1515 },   // ANAIP_PADGPIOCFG4
                { ANAIP_PADGPIOCFG5, 0x1515 },   // ANAIP_PADGPIOCFG5
                { ANAIP_PADGPIOCFG6, 0x1215 },   // ANAIP_PADGPIOCFG6
                { ANAIP_PADGPIOCFG7, 0x1515 },   // ANAIP_PADGPIOCFG7
                { ANAIP_PADGPIOCFG8, 0x1515 },   // ANAIP_PADGPIOCFG8
                { ANAIP_PADGPIOCFG9, 0x0415 },   // ANAIP_PADGPIOCFG9
                { ANAIP_PADGPIOCFG10, 0x0404 },   // ANAIP_PADGPIOCFG10
                { ANAIP_PADGPIOCFG11, 0x0004 },   // ANAIP_PADGPIOCFG11
                { ANAIP_PSUPCFG, 0x0004 },   // ANAIP_PSUPCFG
                { ANAIP_PSUPBODCFG, 0x0000 },   // ANAIP_PSUPBODCFG
                { ANAIP_DPHYPLLPWD, 0x0400 },   // ANAIP_DPHYPLLPWD
                { ANAIP_DPHYPHYCFG3, 0x2124 },   // ANAIP_DPHYCFG3     // A11 - MIPI impedance optimization
                { ANAIP_PSPADCFG, 0x1513 },   // ANAIP_PSPADCFG     // \todo: ROYAL-1504 ANAIP_PSPADCFG earlier
                { CFGCNT_ROS1, 0x0006 },   // CFGCNT_ROS1
                { CFGCNT_ROS2, 0x0060 },   // CFGCNT_ROS2
                { CFGCNT_EXPCFG1, 0x0380 },   // CFGCNT_EXPCFG1
                { CFGCNT_EXPCFG2, 0x000F },   // CFGCNT_EXPCFG2
                { CFGCNT_EXPCFG3, 0x1E0A },   // CFGCNT_EXPCFG3
                { CFGCNT_EXPCFG4, 0x1C05 },   // CFGCNT_EXPCFG4
            };

            const std::map < uint16_t, uint16_t > BaseConfigForEvalboardMIPIBridge
            {
                { MTCU_POWERCTRL, 0x13FF },   //MTCU_POWERCTRL
                { ANAIP_GPIOMUX5, 0x0180 },   // ANAIP_GPIOMUX5     // for Eval Kit Illumination Board Control
                { ANAIP_GPIOMUX6, 0x3140 },   // ANAIP_GPIOMUX6     // for Eval Kit Illumination Board Control
                { ANAIP_GPIOMUX7, 0x018D },   // ANAIP_GPIOMUX7     // for Eval Kit Illumination Board Control
                { ANAIP_PADGPIOCFG0, 0x1313 },   // ANAIP_PADGPIOCFG0
                { ANAIP_PADGPIOCFG1, 0x1313 },   // ANAIP_PADGPIOCFG1
                { ANAIP_PADGPIOCFG2, 0x1313 },   // ANAIP_PADGPIOCFG2
                { ANAIP_PADGPIOCFG3, 0x1313 },   // ANAIP_PADGPIOCFG3
                { ANAIP_PADGPIOCFG4, 0x1313 },   // ANAIP_PADGPIOCFG4
                { ANAIP_PADGPIOCFG5, 0x1313 },   // ANAIP_PADGPIOCFG5
                { ANAIP_PADGPIOCFG6, 0x1313 },   // ANAIP_PADGPIOCFG6
                { ANAIP_PADGPIOCFG7, 0x1313 },   // ANAIP_PADGPIOCFG7
                { ANAIP_PADGPIOCFG8, 0x1313 },   // ANAIP_PADGPIOCFG8
                { ANAIP_PADGPIOCFG9, 0x0413 },   // ANAIP_PADGPIOCFG9
                { ANAIP_PADGPIOCFG10, 0x0404 },   // ANAIP_PADGPIOCFG10
                { ANAIP_PADGPIOCFG11, 0x0004 },   // ANAIP_PADGPIOCFG11
                { ANAIP_PSUPCFG, 0x0004 },   // ANAIP_PSUPCFG
                { ANAIP_PSUPBODCFG, 0x0000 },   // ANAIP_PSUPBODCFG
                { ANAIP_DPHYPLLPWD, 0x0400 },   // ANAIP_DPHYPLLPWD
                { ANAIP_DPHYPHYCFG3, 0x2124 },   // ANAIP_DPHYCFG3     // A11 - MIPI impedance optimization
                { CFGCNT_ROS1, 0x0006 },   // CFGCNT_ROS1
                { CFGCNT_ROS2, 0x0060 },   // CFGCNT_ROS2
                { CFGCNT_EXPCFG1, 0x0380 },   // CFGCNT_EXPCFG1
                { CFGCNT_EXPCFG2, 0x000F },   // CFGCNT_EXPCFG2
                { CFGCNT_EXPCFG3, 0x1E0A },   // CFGCNT_EXPCFG3
                { CFGCNT_EXPCFG4, 0x1C05 },   // CFGCNT_EXPCFG4

                // ROYAL-1315 MIPI compliance
                { ANAIP_DPHYDLANE1CFG1, 0x01c5 },
                { ANAIP_DPHYDLANE2CFG1, 0x01c5 },
                { ANAIP_DPHYDLANE1CFG3, 0x0206 },
                { ANAIP_DPHYDLANE2CFG3, 0x0206 },
            };
        }
    }
}
