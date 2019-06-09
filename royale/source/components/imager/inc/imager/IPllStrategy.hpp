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

#include <vector>
#include <cstdint>

namespace royale
{
    namespace imager
    {
        /**
        * Interface for a PLL calculation
        */
        class IPllStrategy
        {
        public:
            /**
            * Structure for results of function \ref reversePllSettings
            */
            struct ReversePllSettings
            {
                double outFreq;
                double vcoFreq;
                double divNFreq;
                double divMFreq;
                double sscFreq;
                double sscDelta;
                double sscKspread;
                double refIntClkFreq;
                double enableWavegen;
                double enSdm;
                double enConstSdm;
                double constSdm;
                double inputDiv2en;
                double minipeak;
                double increment;
                double cyclesby2;
                double en_ext_mmd_div_ratio;
                double ext_mmd_div_ratio;
                double mmd_in;
                double mod_out_freq_sel;
            };

            virtual ~IPllStrategy() = default;

            /**
            * Calculates the registers for setting the PLL to a certain frequency.
            * Calculation is using double precision to achieve fractional values differing not more then
            * 1d from the MATLAB results (float would have 8 decimal max. deviation)
            * \param   seeking_frequency   The pll seeking frequency. For the DPHY PLL this is the actual frequency of it.
            *                              Note: For the Modulation PLL this is not the illumination frequency -
            *                              there is a clock divider that finally defines the modulation frequency.
            *                              Depending on the concrete imagers different clock dividers may be used.
            *                              If for example a clock divider of four is used the seeking_frequency must
            *                              be set to 400 MHz to get an illumination modulated with a frequency of 100 MHz.
            * \param   enable_wavegen      If true this enables the PLL SSC feature. If the concrete implementation does
            *                              not support this the method will return false. It is up to the concrete implementation
            *                              if also the SSC registers are calculated. If it is supported and they should be
            *                              calculated the provided pllcfg parameter must have a size of 8.
            * \param   pllcfg              The calculated PLL register settings.
            * \param   fssc                input parameter taken into account only if enable_wavegen is true
            * \param   kspread             input parameter taken into account only if enable_wavegen is true
            * \param   delta               input parameter taken into account only if enable_wavegen is true
            * \return True if the frequency setting is feasible, false otherwise.
            */
            virtual bool pllSettings (double seeking_frequency, bool enable_wavegen, std::vector<uint16_t> &pllcfg,
                                      double fssc = 0, double kspread = 0, double delta = 0) = 0;

            /**
            * Calculates human readable frequency and SSC values from register settings.
            * \param    pllcfg             register values
            * \param    outdata            output structure of calculated items
            * \return                      true if calculation succeeded
            */
            virtual bool reversePllSettings (const std::vector<uint16_t> &pllcfg, ReversePllSettings &outdata) = 0;
        };
    }
}
