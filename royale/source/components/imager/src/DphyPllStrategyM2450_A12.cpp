/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/DphyPllStrategyM2450_A12.hpp>
#include <common/exceptions/LogicError.hpp>
#include <algorithm>
#include <cmath>

using namespace royale::common;
using namespace royale::imager;

DphyPllStrategyM2450_A12::DphyPllStrategyM2450_A12 (const uint32_t systemFrequency) :
    m_systemFrequency (systemFrequency)
{

}


bool DphyPllStrategyM2450_A12::dphyPllForbiddenFrequency (double fref_int, double fvco_dphy, double dphypll_frequency)
{
    double temp = std::fmod (fvco_dphy, fref_int);
    double margin = std::min (temp, fref_int - temp);
    double temp_half = std::fmod (fvco_dphy, fref_int / 2.0);
    double margin_half = std::min (temp_half, fref_int / 2.0 - temp_half);

    //forbidden_seeking
    if (margin <= 500000.0 || margin_half <= 500000.0)
    {
        return true;
    }

    //forbidden_limit
    if (fvco_dphy > 960e6 || dphypll_frequency < 200e6)
    {
        return true;
    }

    return false;
}

bool DphyPllStrategyM2450_A12::pllSettings (double seeking_frequency, bool enable_wavegen, std::vector<uint16_t> &pllcfg,
        double fssc, double kspread, double delta)
{
    double ref_clk_int, fvco, base_div_ratio;
    double divmod_frac58, swallow_frac58;
    double divmod_frac, swallow_frac;
    double swallow_minus_4, swallow_minus_4_scaled;
    double ctr_preset, const_sdm;
    uint16_t en_const_sdm, cp_i_sel, pll_ensdm;
    uint16_t input_div_2_en, enable_const, enable_ext;
    uint16_t ext_mmd_div_ratio, mod_out_freq_sel, mmd_in;
    uint32_t const_sdm_24;
    bool bad_ssc_values = false;
    bool is_fractional;

    if (m_systemFrequency <= 20.0 * 1e6)
    {
        ref_clk_int = static_cast<double> (m_systemFrequency);
        input_div_2_en = 0;
    }
    else
    {
        ref_clk_int = static_cast<double> (m_systemFrequency) / 2.0;
        input_div_2_en = 1;
    }

    fvco = seeking_frequency * 2.0;
    base_div_ratio = fvco / ref_clk_int;

    divmod_frac58 = 18.0 + 2.0 * std::floor ( (base_div_ratio - 19.0) / 2.0);
    swallow_frac58 = 2.0 * fmod (base_div_ratio, divmod_frac58);

    if (swallow_frac58 > 5.8)
    {
        divmod_frac = divmod_frac58 + 2.0;
        swallow_frac = 2.0 * fmod (base_div_ratio, divmod_frac);
        if (enable_wavegen)
        {
            // no ssc possible
            bad_ssc_values = true;
        }
    }
    else
    {
        divmod_frac = divmod_frac58;
        swallow_frac = swallow_frac58;
    }

    swallow_minus_4 = swallow_frac - 4.0;
    swallow_minus_4_scaled = swallow_minus_4 * ::exp2 (21.0);
    ctr_preset = divmod_frac / 2.0 - 2.0;
    const_sdm = swallow_minus_4_scaled;

    if (const_sdm < 0.0)
    {
        const_sdm_24 = 0xFFFFFF - (static_cast<uint32_t> (-1.0 * std::floor (const_sdm)) & 0x7FFFFF) + 1;
    }
    else
    {
        const_sdm_24 = static_cast<uint32_t> (std::floor (const_sdm)) & 0x7FFFFF;
    }

    if (fmod (base_div_ratio, 0.5) != 0)
    {
        is_fractional = true;
    }
    else
    {
        is_fractional = false;
    }

    if (enable_wavegen)
    {
        enable_ext = 0;
        enable_const = 0;
    }
    else
    {
        if (is_fractional)
        {
            enable_ext = 0;
            enable_const = 1;
        }
        else
        {
            enable_ext = 1;
            enable_const = 0;
        }
    }

    mmd_in = static_cast<uint16_t> (ctr_preset);
    ext_mmd_div_ratio = static_cast<uint16_t> (swallow_frac);
    mod_out_freq_sel = static_cast<uint16_t> (0);

    if (enable_ext)
    {
        pll_ensdm = 0;
        en_const_sdm = 0;
    }
    else
    {
        pll_ensdm = 1;
        en_const_sdm = enable_const;
    }

    cp_i_sel = calcChargePumpCurrent (ref_clk_int, fvco);

    if (enable_wavegen)
    {
        if (fssc <= 0.0)
        {
            throw LogicError ("fssc must be greater than zero");
        }

        double fvco_min_possible = ref_clk_int * (2.0 * (ctr_preset + 2.0) + 0.5);
        double fvco_max_possible = ref_clk_int * (2.0 * (ctr_preset + 2.0) + 3.0);
        double fvco_min = fvco * (1.0 + delta * (kspread - 1.0));
        double fvco_max = fvco * (1.0 + kspread * delta);

        if (fvco_min_possible > fvco_min || fvco_max_possible < fvco_max)
        {
            // no ssc possible frequency to high/low
            bad_ssc_values = true;
        }

        // calculate SSC registers as well
        double min_base_div_ratio = fvco * (1.0 - delta + kspread * delta) / ref_clk_int;
        double min_swallow = 2.0 * fmod (min_base_div_ratio, divmod_frac58);
        double min_swallow_minus_4 = min_swallow - 4.0;
        double minipeak = min_swallow_minus_4 * ::exp2 (21.0);
        int32_t minipeak_bin = static_cast<int32_t> (trunc (minipeak)); // 24bits
        int32_t x = (1 << (24 - 1));
        if (minipeak_bin > (x - 1))
        {
            minipeak_bin = x - 1;
        }
        if (minipeak_bin < -x)
        {
            minipeak_bin = -x;
        }

        double increment_num = fvco * delta / ref_clk_int;
        double increment_den = ref_clk_int / (2.0 * fssc);
        uint32_t increment_bin = static_cast<uint32_t> (trunc (increment_num * ::exp2 (21.0) / increment_den * 2.0)); // 16bits
        if (increment_bin > ( (1 << 16) - 1))
        {
            increment_bin = (1 << 16) - 1;
        }

        double cyclesby2 = ref_clk_int / (2.0 * fssc) - 1.0;
        uint16_t cyclesby2_bin = static_cast<uint16_t> (trunc (cyclesby2)); // 14bits
        if (cyclesby2_bin > ( (1 << 14) - 1))
        {
            cyclesby2_bin = (1 << 14) - 1;
        }

        uint64_t pll_sscmod_bin = cyclesby2_bin;
        pll_sscmod_bin = (pll_sscmod_bin << 16) | increment_bin;
        pll_sscmod_bin = (pll_sscmod_bin << 24) | (minipeak_bin & ( (1 << 24) - 1));

        if (pllcfg.size() >= 6)
        {
            auto pll_sscmod_bin_slice = [ = ] (uint16_t to, uint16_t from)
            {
                return static_cast<uint16_t> ( (pll_sscmod_bin >> from) & ( (1 << (to - from + 1)) - 1));
            };

            pllcfg.at (2) = pll_sscmod_bin_slice (15, 0);
            pllcfg.at (3) = pll_sscmod_bin_slice (31, 16);
            pllcfg.at (4) = pll_sscmod_bin_slice (47, 32);
            pllcfg.at (5) = pll_sscmod_bin_slice (53, 48);
        }

        // Checking limits for SSC parameters:
        // Fss: 0 .. 100kHz
        if (fssc > 100000.0)
        {
            bad_ssc_values = true;
        }
    }

    if (pllcfg.size() >= 2)
    {
        const uint16_t lf_res_sel = 0;
        const uint16_t boost_up_en = 1;
        const uint16_t boost_up_maxcurr_en = 0;
        const uint16_t cp_force_fix_bias = 0;
        const uint16_t current_sel_testopa = 1;
        const uint16_t current_sel_lfopa = 1;
        const uint16_t current_sel_itest = 1;
        const uint16_t current_sel_cpota = 1;
        const uint16_t cp_ref_sel = 5;

        pllcfg.at (0) = static_cast<uint16_t> (ext_mmd_div_ratio << 13 |
                                               enable_ext << 12 |
                                               mmd_in << 5 |
                                               lf_res_sel << 4 |
                                               cp_force_fix_bias << 3 |
                                               boost_up_maxcurr_en << 2 |
                                               input_div_2_en << 1 |
                                               boost_up_en << 0);
        pllcfg.at (1) = static_cast<uint16_t> (current_sel_testopa << 14 |
                                               current_sel_lfopa << 12 |
                                               current_sel_itest << 10 |
                                               current_sel_cpota << 8 |
                                               cp_i_sel << 6 |
                                               cp_ref_sel << 3 |
                                               mod_out_freq_sel);
    }
    if (pllcfg.size() >= 8)
    {
        pllcfg.at (6) = static_cast<uint16_t> (const_sdm_24 & 0xFFFF);
        pllcfg.at (7) = static_cast<uint16_t> ( (enable_wavegen ? 1 : 0) << 10 |
                                                pll_ensdm << 9 |
                                                en_const_sdm << 8 |
                                                ( (const_sdm_24 >> 16) & 0xFF));
    }

    return !bad_ssc_values && !dphyPllForbiddenFrequency (ref_clk_int, fvco, seeking_frequency);
}

uint16_t DphyPllStrategyM2450_A12::calcChargePumpCurrent (double ref_clk_int, double fvco)
{
    uint16_t cp_i_sel;

    if (ref_clk_int <= 15e6)
    {
        if (fvco < 570e6)
        {
            cp_i_sel = 0;
        }
        else if (fvco >= 570e6 && fvco < 800e6)
        {
            cp_i_sel = 1;
        }
        else
        {
            cp_i_sel = 2;
        }
    }
    else
    {
        if (fvco < 650e6)
        {
            cp_i_sel = 0;
        }
        else
        {
            cp_i_sel = 1;
        }
    }

    return cp_i_sel;
}

bool DphyPllStrategyM2450_A12::reversePllSettings (const std::vector<uint16_t> &pllcfg, ReversePllSettings &outdata)
{
    bool success = true;

    if (pllcfg.size() < 8)
    {
        throw LogicError ("register list must contain minimum 8 registers");
    }

    auto bitfield = [ = ] (uint32_t reg, uint16_t from, uint16_t width, bool isSigned)
    {
        int32_t s = (reg >> from) & ( (1 << width) - 1);

        // if signed and sign bit is set, then number is negative
        if (isSigned && (s & (1 << (width - 1))))
        {
            s |= ( (static_cast<int32_t> (1) << (32 - width)) - 1) << width;
        }
        return static_cast<double> (s);
    };

    const uint16_t reg1 = pllcfg.at (0);
    double input_div_2_en = bitfield (reg1, 1, 1, false);
    double en_ext_mmd_div_ratio = bitfield (reg1, 12, 1, false);
    double ext_mmd_div_ratio_dec = bitfield (reg1, 13, 3, false);
    double mmd_in_dec = bitfield (reg1, 5, 7, false);
    double mod_out_freq_sel_dec = 2.0;

    const uint32_t const_sdm_hi = pllcfg.at (7);
    double const_sdm = bitfield (const_sdm_hi << 16 | pllcfg.at (6), 0, 24, true);
    double en_const_sdm = bitfield (const_sdm_hi, 8, 1, false);
    double en_sdm = bitfield (const_sdm_hi, 9, 1, false);
    double enable_wavegen = bitfield (const_sdm_hi, 10, 1, false);

    double ref_clk_int = static_cast<double> (m_systemFrequency) / exp2 (input_div_2_en);
    double swallow_frac = const_sdm / exp2 (21.0) + 4.0;

    double div;
    if (en_ext_mmd_div_ratio == 1.0)
    {
        div = ext_mmd_div_ratio_dec / 2. + 2. * mmd_in_dec + 4.;
    }
    else
    {
        div = swallow_frac / 2. + 2. * mmd_in_dec + 4.;
    }

    double modpll_vco_frequency = div * ref_clk_int;
    double modpll_out_frequency = modpll_vco_frequency / mod_out_freq_sel_dec;
    double phaseshifter_divn_frequency = modpll_out_frequency / 3.0; // sys clk
    double phaseshifter_divm_frequency = modpll_out_frequency / 4.0; // esc clk

    uint64_t pll_sscmod_bin = pllcfg.at (5) & ( (1 << 6) - 1);
    pll_sscmod_bin = (pll_sscmod_bin << 16) | pllcfg.at (4);
    pll_sscmod_bin = (pll_sscmod_bin << 16) | pllcfg.at (3);
    pll_sscmod_bin = (pll_sscmod_bin << 16) | pllcfg.at (2);

    auto pll_sscmod_bin_slice_r = [ = ] (uint16_t from, uint16_t to)
    {
        return static_cast<uint32_t> ( (pll_sscmod_bin >> (54 - to)) & ( (1 << (to - from + 1)) - 1));
    };

    double minipeak_dec = bitfield (pll_sscmod_bin_slice_r (31, 54), 0, 24, true);
    double increment_dec = pll_sscmod_bin_slice_r (15, 30);
    double cyclesby2_dec = pll_sscmod_bin_slice_r (1, 14);

    double min_swallow = minipeak_dec / exp2 (21.0) + 4.0;
    double divmod = 18.0 + 2.0 * floor ( (div - 19.0) / 2.0);
    double mod = min_swallow / 2.0;
    double min_base_div_ratio = mod + divmod;

    double fssc = ref_clk_int / ( (cyclesby2_dec + 1.0) * 2.0);
    double delta = pow (ref_clk_int, 2.0) * increment_dec / (exp2 (23.0) * modpll_vco_frequency * fssc);
    double kspread = (min_base_div_ratio * ref_clk_int / modpll_vco_frequency - 1.0 + delta) / delta;

    outdata.outFreq         = modpll_out_frequency;
    outdata.vcoFreq         = modpll_vco_frequency;
    outdata.divNFreq        = phaseshifter_divn_frequency;
    outdata.divMFreq        = phaseshifter_divm_frequency;
    outdata.sscFreq         = fssc;
    outdata.sscDelta        = delta;
    outdata.sscKspread      = kspread;
    outdata.refIntClkFreq   = ref_clk_int;
    outdata.enableWavegen   = enable_wavegen;
    outdata.enSdm = en_sdm;
    outdata.enConstSdm = en_const_sdm;
    outdata.constSdm        = const_sdm;
    outdata.inputDiv2en     = input_div_2_en;
    outdata.minipeak        = minipeak_dec;
    outdata.increment       = increment_dec;
    outdata.cyclesby2       = cyclesby2_dec;
    outdata.en_ext_mmd_div_ratio = en_ext_mmd_div_ratio;
    outdata.ext_mmd_div_ratio = ext_mmd_div_ratio_dec;
    outdata.mmd_in = mmd_in_dec;
    outdata.mod_out_freq_sel = mod_out_freq_sel_dec;

    return success;
}
