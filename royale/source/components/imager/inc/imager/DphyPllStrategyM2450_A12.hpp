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

#include <imager/ImagerCommon.hpp>
#include <imager/IPllStrategy.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * Interface for a PLL calculation
        */
        class DphyPllStrategyM2450_A12 : public IPllStrategy
        {
        public:
            IMAGER_EXPORT explicit DphyPllStrategyM2450_A12 (const uint32_t systemFrequency);

            /**
            * Calculates the registers for setting the DPHY PLL to a certain frequency.
            * Calculations follow the single source at:
            * //project1.vih.infineon.com/project1/DCGRWS_Projects/ToF/MiraCE/03_development/
            * 3_1_Analog_Macros/07_macro_pll/CE/mtatlab_setting_calculations/dphy_pll_settings_A12.m
            * Deviation from original calculation:
            *  - manual override is not allowed, discard all if(man_override==1) clauses
            */
            bool pllSettings (double seeking_frequency, bool enable_wavegen, std::vector<uint16_t> &pllcfg,
                              double fssc = 0, double kspread = 0, double delta = 0) override;

            bool reversePllSettings (const std::vector<uint16_t> &pllcfg, ReversePllSettings &outdata) override;

        private:
            uint16_t calcChargePumpCurrent (double ref_clk_int, double fvco);

            bool dphyPllForbiddenFrequency (double fref_int, double fvco_dphy, double dphypll_frequency);

            const uint32_t m_systemFrequency;
        };
    }
}
