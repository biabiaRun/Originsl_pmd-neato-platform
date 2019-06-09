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
        * DPHY PLL calculation for a M2452 imager
        */
        class DphyPllStrategyM2452 : public IPllStrategy
        {
        public:
            IMAGER_EXPORT explicit DphyPllStrategyM2452 (const uint32_t systemFrequency);

            /**
            * Calculates the registers for setting the DPHY PLL to a certain frequency.
            * Calculations follow the single source at:
            * https://ifxsvnwn.muc.infineon.com/svn/mirace_opt_dev/trunk/Single_Sources/PLL_calculations/MIRALOTTE_dphy_pll_settings_core.m
            */
            bool pllSettings (double seeking_frequency, bool enable_wavegen, std::vector<uint16_t> &pllcfg,
                              double fssc = 0, double kspread = 0, double delta = 0) override;

            bool reversePllSettings (const std::vector<uint16_t> &pllcfg, ReversePllSettings &outdata) override;

        private:
            const uint32_t m_systemFrequency;

            bool dphyPllForbiddenFrequency (double fref_int, double fvco_dphy, double dphypll_frequency);

            /*
            * Calculates the setting for the register slice that sets the charge pump current.
            */
            uint16_t calcChargePumpCurrent (double ref_clk_int, double fvco);
        };
    }
}
