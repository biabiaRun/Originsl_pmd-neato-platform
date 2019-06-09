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

#include <imager/M2450_A11/ImagerRegisters.hpp>

#include <map>

namespace royale
{
    namespace imager
    {
        namespace M2450_A11
        {
            const std::map < uint16_t, uint16_t > BaseConfig
            {
                { CFGCNT_S00_EXPOTIME, 0x0E68 },
                { CFGCNT_S00_FRAMERATE, 0x0000 },
                { CFGCNT_S00_PS, 0x0000 },
                { CFGCNT_S00_PLLSET, 0x0000 },
                { CFGCNT_S01_EXPOTIME, 0x0E68 },
                { CFGCNT_S01_FRAMERATE, 0x0000 },
                { CFGCNT_S01_PS, 0x0888 },
                { CFGCNT_S01_PLLSET, 0x0000 },
                { CFGCNT_S02_EXPOTIME, 0x0E68 },
                { CFGCNT_S02_FRAMERATE, 0x0000 },
                { CFGCNT_S02_PS, 0x0444 },
                { CFGCNT_S02_PLLSET, 0x0000 },
                { CFGCNT_S03_EXPOTIME, 0x0E68 },
                { CFGCNT_S03_FRAMERATE, 0x0000 },
                { CFGCNT_S03_PS, 0x0CCC },
                { CFGCNT_S03_PLLSET, 0x0000 },
                { CFGCNT_S04_EXPOTIME, 0x0000 },
                { CFGCNT_S04_FRAMERATE, 0x0000 },
                { CFGCNT_S04_PS, 0x0000 },
                { CFGCNT_S04_PLLSET, 0x0000 },
                { CFGCNT_S05_EXPOTIME, 0x0000 },
                { CFGCNT_S05_FRAMERATE, 0x0000 },
                { CFGCNT_S05_PS, 0x0000 },
                { CFGCNT_S05_PLLSET, 0x0000 },
                { CFGCNT_S06_EXPOTIME, 0x0000 },
                { CFGCNT_S06_FRAMERATE, 0x0000 },
                { CFGCNT_S06_PS, 0x0000 },
                { CFGCNT_S06_PLLSET, 0x0000 },
                { CFGCNT_S07_EXPOTIME, 0x0000 },
                { CFGCNT_S07_FRAMERATE, 0x0000 },
                { CFGCNT_S07_PS, 0x0000 },
                { CFGCNT_S07_PLLSET, 0x0000 },
                { CFGCNT_S08_EXPOTIME, 0x0000 },
                { CFGCNT_S08_FRAMERATE, 0x0000 },
                { CFGCNT_S08_PS, 0x0000 },
                { CFGCNT_S08_PLLSET, 0x0000 },
                { CFGCNT_S09_EXPOTIME, 0x0000 },
                { CFGCNT_S09_FRAMERATE, 0x0000 },
                { CFGCNT_S09_PS, 0x0000 },
                { CFGCNT_S09_PLLSET, 0x0000 },
                { CFGCNT_S10_EXPOTIME, 0x0000 },
                { CFGCNT_S10_FRAMERATE, 0x0000 },
                { CFGCNT_S10_PS, 0x0000 },
                { CFGCNT_S10_PLLSET, 0x0000 },
                { CFGCNT_S11_EXPOTIME, 0x0000 },
                { CFGCNT_S11_FRAMERATE, 0x0000 },
                { CFGCNT_S11_PS, 0x0000 },
                { CFGCNT_S11_PLLSET, 0x0000 },
                { CFGCNT_S12_EXPOTIME, 0x0000 },
                { CFGCNT_S12_FRAMERATE, 0x0000 },
                { CFGCNT_S12_PS, 0x0000 },
                { CFGCNT_S12_PLLSET, 0x0000 },
                { CFGCNT_S13_EXPOTIME, 0x0000 },
                { CFGCNT_S13_FRAMERATE, 0x0000 },
                { CFGCNT_S13_PS, 0x0000 },
                { CFGCNT_S13_PLLSET, 0x0000 },
                { CFGCNT_S14_EXPOTIME, 0x0000 },
                { CFGCNT_S14_FRAMERATE, 0x0000 },
                { CFGCNT_S14_PS, 0x0000 },
                { CFGCNT_S14_PLLSET, 0x0000 },
                { CFGCNT_S15_EXPOTIME, 0x0000 },
                { CFGCNT_S15_FRAMERATE, 0x0000 },
                { CFGCNT_S15_PS, 0x0000 },
                { CFGCNT_S15_PLLSET, 0x0000 },
                { CFGCNT_S16_EXPOTIME, 0x0000 },
                { CFGCNT_S16_FRAMERATE, 0x0000 },
                { CFGCNT_S16_PS, 0x0000 },
                { CFGCNT_S16_PLLSET, 0x0000 },
                { CFGCNT_S17_EXPOTIME, 0x0000 },
                { CFGCNT_S17_FRAMERATE, 0x0000 },
                { CFGCNT_S17_PS, 0x0000 },
                { CFGCNT_S17_PLLSET, 0x0000 },
                { CFGCNT_S18_EXPOTIME, 0x0000 },
                { CFGCNT_S18_FRAMERATE, 0x0000 },
                { CFGCNT_S18_PS, 0x0000 },
                { CFGCNT_S18_PLLSET, 0x0000 },
                { CFGCNT_S19_EXPOTIME, 0x0000 },
                { CFGCNT_S19_FRAMERATE, 0x0000 },
                { CFGCNT_S19_PS, 0x0000 },
                { CFGCNT_S19_PLLSET, 0x0000 },
                { CFGCNT_S20_EXPOTIME, 0x0000 },
                { CFGCNT_S20_FRAMERATE, 0x0000 },
                { CFGCNT_S20_PS, 0x0000 },
                { CFGCNT_S20_PLLSET, 0x0000 },
                { CFGCNT_S21_EXPOTIME, 0x0000 },
                { CFGCNT_S21_FRAMERATE, 0x0000 },
                { CFGCNT_S21_PS, 0x0000 },
                { CFGCNT_S21_PLLSET, 0x0000 },
                { CFGCNT_S22_EXPOTIME, 0x0000 },
                { CFGCNT_S22_FRAMERATE, 0x0000 },
                { CFGCNT_S22_PS, 0x0000 },
                { CFGCNT_S22_PLLSET, 0x0000 },
                { CFGCNT_S23_EXPOTIME, 0x0000 },
                { CFGCNT_S23_FRAMERATE, 0x0000 },
                { CFGCNT_S23_PS, 0x0000 },
                { CFGCNT_S23_PLLSET, 0x0000 },
                { CFGCNT_S24_EXPOTIME, 0x0000 },
                { CFGCNT_S24_FRAMERATE, 0x0000 },
                { CFGCNT_S24_PS, 0x0000 },
                { CFGCNT_S24_PLLSET, 0x0000 },
                { CFGCNT_S25_EXPOTIME, 0x0000 },
                { CFGCNT_S25_FRAMERATE, 0x0000 },
                { CFGCNT_S25_PS, 0x0000 },
                { CFGCNT_S25_PLLSET, 0x0000 },
                { CFGCNT_S26_EXPOTIME, 0x0000 },
                { CFGCNT_S26_FRAMERATE, 0x0000 },
                { CFGCNT_S26_PS, 0x0000 },
                { CFGCNT_S26_PLLSET, 0x0000 },
                { CFGCNT_S27_EXPOTIME, 0x0000 },
                { CFGCNT_S27_FRAMERATE, 0x0000 },
                { CFGCNT_S27_PS, 0x0000 },
                { CFGCNT_S27_PLLSET, 0x0000 },
                { CFGCNT_S28_EXPOTIME, 0x0000 },
                { CFGCNT_S28_FRAMERATE, 0x0000 },
                { CFGCNT_S28_PS, 0x0000 },
                { CFGCNT_S28_PLLSET, 0x0000 },
                { CFGCNT_S29_EXPOTIME, 0x0000 },
                { CFGCNT_S29_FRAMERATE, 0x0000 },
                { CFGCNT_S29_PS, 0x0000 },
                { CFGCNT_S29_PLLSET, 0x0000 },
                { CFGCNT_S30_EXPOTIME, 0x0000 },
                { CFGCNT_S30_FRAMERATE, 0x0000 },
                { CFGCNT_S30_PS, 0x0000 },
                { CFGCNT_S30_PLLSET, 0x0000 },
                { CFGCNT_S31_EXPOTIME, 0x0000 },
                { CFGCNT_S31_FRAMERATE, 0x0000 },
                { CFGCNT_S31_PS, 0x0000 },
                { CFGCNT_S31_PLLSET, 0x0000 },
                { CFGCNT_BINCFG, 0x0004 },
                { CFGCNT_ROICMINREG, 0x0000 },
                { CFGCNT_ROICMAXREG, 0x015F },
                { CFGCNT_ROIRMINREG, 0x0000 },
                { CFGCNT_ROIRMAXREG, 0x011F },
                { CFGCNT_CTRLSEQ, 0x0003 },

                //LUT1 = 30MHz
                { CFGCNT_PLLCFG1_LUT1, 0x2241 },   // CFGCNT_PLLCFG1_LUT1
                { CFGCNT_PLLCFG2_LUT1, 0x13B2 },   // CFGCNT_PLLCFG2_LUT1
                { CFGCNT_PLLCFG3_LUT1, 0x03BB },   // CFGCNT_PLLCFG3_LUT1

                //LUT2 = 60MHz
                { CFGCNT_PLLCFG1_LUT2, 0x2041 },
                { CFGCNT_PLLCFG2_LUT2, 0x13B1 },
                { CFGCNT_PLLCFG3_LUT2, 0x03BB },

                //LUT3 = 20MHz (warning: suboptimal frequency used)
                { CFGCNT_PLLCFG1_LUT3, 0x4459 },
                { CFGCNT_PLLCFG2_LUT3, 0xC4EC },
                { CFGCNT_PLLCFG3_LUT3, 0x0BCE },

                //LUT4 = 99.5MHz
                // already defined by reset config
                { CFGCNT_PLLCFG1_LUT4, 0x4071 },
                { CFGCNT_PLLCFG2_LUT4, 0xC4ED },
                { CFGCNT_PLLCFG3_LUT4, 0x13CE }
            };
        }
    }
}
