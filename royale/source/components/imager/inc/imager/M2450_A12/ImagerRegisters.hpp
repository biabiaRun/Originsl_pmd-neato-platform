/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

namespace royale
{
    namespace imager
    {
        namespace M2450_A12
        {
            const uint16_t ANAIP_SENSREFI = 0x0B000;
            const uint16_t ANAIP_SENSAMUX = 0x0B001;
            const uint16_t ANAIP_SENSEN = 0x0B002;
            const uint16_t ANAIP_GPIOMUX0 = 0x0B003;
            const uint16_t ANAIP_GPIOMUX1 = 0x0B004;
            const uint16_t ANAIP_GPIOMUX2 = 0x0B005;
            const uint16_t ANAIP_GPIOMUX3 = 0x0B006;
            const uint16_t ANAIP_GPIOMUX4 = 0x0B007;
            const uint16_t ANAIP_GPIOMUX5 = 0x0B008;
            const uint16_t ANAIP_GPIOMUX6 = 0x0B009;
            const uint16_t ANAIP_GPIOMUX7 = 0x0B00A;
            const uint16_t ANAIP_MODPADMUX = 0x0B00B;
            const uint16_t ANAIP_GPIO_CLK_CFG1 = 0x0B00C;
            const uint16_t ANAIP_GPIO_CLK_CFG2 = 0x0B00D;
            const uint16_t ANAIP_PADGPIOCFG0 = 0x0B00E;
            const uint16_t ANAIP_PADGPIOCFG1 = 0x0B00F;
            const uint16_t ANAIP_PADGPIOCFG2 = 0x0B010;
            const uint16_t ANAIP_PADGPIOCFG3 = 0x0B011;
            const uint16_t ANAIP_PADGPIOCFG4 = 0x0B012;
            const uint16_t ANAIP_PADGPIOCFG5 = 0x0B013;
            const uint16_t ANAIP_PADGPIOCFG6 = 0x0B014;
            const uint16_t ANAIP_PADGPIOCFG7 = 0x0B015;
            const uint16_t ANAIP_PADGPIOCFG8 = 0x0B016;
            const uint16_t ANAIP_PADGPIOCFG9 = 0x0B017;
            const uint16_t ANAIP_PADGPIOCFG10 = 0x0B018;
            const uint16_t ANAIP_PADGPIOCFG11 = 0x0B019;
            const uint16_t ANAIP_PADSDASCL = 0x0B01A;
            const uint16_t ANAIP_EFUSEPSWRD = 0x0B01B;
            const uint16_t ANAIP_ADCBG1 = 0x0B01C;
            const uint16_t ANAIP_ADCBG2 = 0x0B01D;
            const uint16_t ANAIP_ADCEN = 0x0B01E;
            const uint16_t ANAIP_ADCRESET = 0x0B01F;
            const uint16_t ANAIP_ADCCFG1 = 0x0B020;
            const uint16_t ANAIP_ADCCFG2 = 0x0B021;
            const uint16_t ANAIP_ADCBIST1CFG1 = 0x0B022;
            const uint16_t ANAIP_ADCBIST1CFG2 = 0x0B023;
            const uint16_t ANAIP_ADCBIST1CFG3 = 0x0B024;
            const uint16_t ANAIP_ADCBIST1CFG4 = 0x0B025;
            const uint16_t ANAIP_ADCBIST1AMUX = 0x0B026;
            const uint16_t ANAIP_ADCBIST2CFG1 = 0x0B027;
            const uint16_t ANAIP_ADCBIST2CFG2 = 0x0B028;
            const uint16_t ANAIP_ADCBIST2CFG3 = 0x0B029;
            const uint16_t ANAIP_ADCBIST2CFG4 = 0x0B02A;
            const uint16_t ANAIP_ADCBIST2AMUX = 0x0B02B;
            const uint16_t ANAIP_ADCTESTCTRL = 0x0B02C;
            const uint16_t ANAIP_ADCTEST0 = 0x0B02D;
            const uint16_t ANAIP_ADCTEST1 = 0x0B02E;
            const uint16_t ANAIP_ADCBIST1MUX_EVEN_S0 = 0x0B030;
            const uint16_t ANAIP_ADCBIST1MUX_ODD_S0 = 0x0B031;
            const uint16_t ANAIP_ADCBIST2MUX_EVEN_S0 = 0x0B032;
            const uint16_t ANAIP_ADCBIST2MUX_ODD_S0 = 0x0B033;
            const uint16_t ANAIP_ADCBIST1MUX_EVEN_S1 = 0x0B034;
            const uint16_t ANAIP_ADCBIST1MUX_ODD_S1 = 0x0B035;
            const uint16_t ANAIP_ADCBIST2MUX_EVEN_S1 = 0x0B036;
            const uint16_t ANAIP_ADCBIST2MUX_ODD_S1 = 0x0B037;
            const uint16_t ANAIP_PIXIF1 = 0x0B038;
            const uint16_t ANAIP_PIXIF2 = 0x0B039;
            const uint16_t ANAIP_PIXIF3 = 0x0B03A;
            const uint16_t ANAIP_PIXIF4 = 0x0B03B;
            const uint16_t ANAIP_PIXIF5 = 0x0B03C;
            const uint16_t ANAIP_PIXIF6 = 0x0B03D;
            const uint16_t ANAIP_PIXIF7 = 0x0B03E;
            const uint16_t ANAIP_PIXIF8 = 0x0B03F;
            const uint16_t ANAIP_PIXIFBCS = 0x0B040;
            const uint16_t ANAIP_PIXIFEN = 0x0B041;
            const uint16_t ANAIP_PIXREFBGEN = 0x0B042;
            const uint16_t ANAIP_PIXREFEN = 0x0B043;
            const uint16_t ANAIP_PIXREFTRIM = 0x0B044;
            const uint16_t ANAIP_PIXREFAMUXTEST = 0x0B045;
            const uint16_t ANAIP_PSUPCFG = 0x0B046;
            const uint16_t ANAIP_PSUPBODCFG = 0x0B047;
            const uint16_t ANAIP_PLLBGEN = 0x0B048;
            const uint16_t ANAIP_PLLBG = 0x0B049;
            const uint16_t ANAIP_PLLPWD = 0x0B04A;
            const uint16_t ANAIP_PLLRESET = 0x0B04B;
            const uint16_t ANAIP_PLLCFG1 = 0x0B04C;
            const uint16_t ANAIP_PLLCFG2 = 0x0B04D;
            const uint16_t ANAIP_PLLCFG3 = 0x0B04E;
            const uint16_t ANAIP_PLLCFG4 = 0x0B04F;
            const uint16_t ANAIP_PLLCFG5 = 0x0B050;
            const uint16_t ANAIP_PLLCFG6 = 0x0B051;
            const uint16_t ANAIP_PLLCFG7 = 0x0B052;
            const uint16_t ANAIP_PLLCFG8 = 0x0B053;
            const uint16_t ANAIP_PLLDFT = 0x0B054;
            const uint16_t ANAIP_DPHYCLKCFG1 = 0x0B055;
            const uint16_t ANAIP_DPHYCLKCFG2 = 0x0B056;
            const uint16_t ANAIP_DPHYCLANECFG1 = 0x0B057;
            const uint16_t ANAIP_DPHYCLANECFG2 = 0x0B058;
            const uint16_t ANAIP_DPHYCLANECFG3 = 0x0B059;
            const uint16_t ANAIP_DPHYDLANE1CFG1 = 0x0B05A;
            const uint16_t ANAIP_DPHYDLANE1CFG2 = 0x0B05B;
            const uint16_t ANAIP_DPHYDLANE1CFG3 = 0x0B05C;
            const uint16_t ANAIP_DPHYDLANE2CFG1 = 0x0B05D;
            const uint16_t ANAIP_DPHYDLANE2CFG2 = 0x0B05E;
            const uint16_t ANAIP_DPHYDLANE2CFG3 = 0x0B05F;
            const uint16_t ANAIP_DPHYPLLRESET = 0x0B060;
            const uint16_t ANAIP_DPHYPLLPWD = 0x0B061;
            const uint16_t ANAIP_DPHYPLLCFG1 = 0x0B062;
            const uint16_t ANAIP_DPHYPLLCFG2 = 0x0B063;
            const uint16_t ANAIP_DPHYPLLCFG3 = 0x0B064;
            const uint16_t ANAIP_DPHYPLLCFG4 = 0x0B065;
            const uint16_t ANAIP_DPHYPLLCFG5 = 0x0B066;
            const uint16_t ANAIP_DPHYPLLCFG6 = 0x0B067;
            const uint16_t ANAIP_DPHYPLLCFG7 = 0x0B068;
            const uint16_t ANAIP_DPHYPLLCFG8 = 0x0B069;
            const uint16_t ANAIP_DPHYPHYEN = 0x0B06A;
            const uint16_t ANAIP_DPHYPHYRESET = 0x0B06B;
            const uint16_t ANAIP_DPHYPHYCFG1 = 0x0B06C;
            const uint16_t ANAIP_DPHYPHYCFG2 = 0x0B06D;
            const uint16_t ANAIP_DPHYPHYCFG3 = 0x0B06E;
            const uint16_t ANAIP_DPHYBGEN = 0x0B06F;
            const uint16_t ANAIP_DPHYBGCFG = 0x0B070;
            const uint16_t ANAIP_DPHYDFTPLL1 = 0x0B071;
            const uint16_t ANAIP_DPHYDFTPLL2 = 0x0B072;
            const uint16_t ANAIP_DPHYDFTPHY1 = 0x0B073;
            const uint16_t ANAIP_DPHYDFTPHY2 = 0x0B074;
            const uint16_t ANAIP_DPHYDFTPHYCLANE = 0x0B075;
            const uint16_t ANAIP_DPHYDFTPHYDLANE1CFG1 = 0x0B076;
            const uint16_t ANAIP_DPHYDFTPHYDLANE1CFG2 = 0x0B077;
            const uint16_t ANAIP_DPHYDFTPHYDLANE2CFG1 = 0x0B078;
            const uint16_t ANAIP_DPHYDFTPHYDLANE2CFG2 = 0x0B079;
            const uint16_t ANAIP_TSENS = 0x0B07F;
            const uint16_t ANAIP_PSCFG = 0x0B080;
            const uint16_t ANAIP_PS = 0x0B081;
            const uint16_t ANAIP_PSTESTMODE = 0x0B082;
            const uint16_t ANAIP_SENMUX = 0x0B083;
            const uint16_t ANAIP_SEPMUX = 0x0B084;
            const uint16_t ANAIP_LVDSMUX = 0x0B085;
            const uint16_t ANAIP_PSPADCFG = 0x0B086;
            const uint16_t ANAIP_PSLVDSCFG = 0x0B087;
            const uint16_t ANAIP_CMC = 0x0B088;
            const uint16_t ANAIP_AMUXCH0_UB = 0x0B089;
            const uint16_t ANAIP_AMUXCH0_B = 0x0B08A;
            const uint16_t ANAIP_AMUXCH1_UB = 0x0B08B;
            const uint16_t ANAIP_AMUXCH1_B = 0x0B08C;
            const uint16_t ANAIP_AMUXCH2_UB = 0x0B08D;
            const uint16_t ANAIP_AMUXCH2_B = 0x0B08E;
            const uint16_t ANAIP_AMUXCH3_UB = 0x0B08F;
            const uint16_t ANAIP_AMUXCH3_B = 0x0B090;
            const uint16_t ANAIP_AMUXCH4_UB = 0x0B091;
            const uint16_t ANAIP_AMUXCH4_B = 0x0B092;
            const uint16_t ANAIP_AMUXCH5_UB = 0x0B093;
            const uint16_t ANAIP_AMUXCH5_B = 0x0B094;
            const uint16_t ANAIP_AMUXCH6_UB = 0x0B095;
            const uint16_t ANAIP_AMUXCH6_B = 0x0B096;
            const uint16_t ANAIP_AMUXCH7_UB = 0x0B097;
            const uint16_t ANAIP_AMUXCH7_B = 0x0B098;
            const uint16_t ANAIP_AMUXCFG = 0x0B099;
            const uint16_t ANAIP_AMUXPADEN = 0x0B09A;
            const uint16_t ANAIP_EFUSECTRL = 0x0B09B;
            const uint16_t ANAIP_EFUSECLK = 0x0B09C;
            const uint16_t ANAIP_EFUSESTREAM = 0x0B09D;
            const uint16_t ANAIP_EFUSEVAL1 = 0x0B09E;
            const uint16_t ANAIP_EFUSEVAL2 = 0x0B09F;
            const uint16_t ANAIP_EFUSEVAL3 = 0x0B0A0;
            const uint16_t ANAIP_EFUSEVAL4 = 0x0B0A1;
            const uint16_t ANAIP_VMODREG = 0x0B0A2;
            const uint16_t ANAIP_FMUCTRL = 0x0B0A3;
            const uint16_t ANAIP_FMUCFG1 = 0x0B0A4;
            const uint16_t ANAIP_FMUCFG2 = 0x0B0A5;
            const uint16_t ANAIP_MBISTCFG = 0x0B0A8;
            const uint16_t ANAIP_I2CADDR = 0x0B0AC;
            const uint16_t ANAIP_DESIGNSTEP = 0x0B0AD;
            const uint16_t ANAIP_SPARE = 0x0B0AE;
            const uint16_t BINNING_BINCFG = 0x0A000;
            const uint16_t BINNING_HSIZE = 0x0A001;
            const uint16_t BINNING_SPARE = 0x0A004;
            const uint16_t CFGCNT_S00_EXPOTIME = 0x0A800;
            const uint16_t CFGCNT_S00_FRAMERATE = 0x0A801;
            const uint16_t CFGCNT_S00_PS = 0x0A802;
            const uint16_t CFGCNT_S00_PLLSET = 0x0A803;
            const uint16_t CFGCNT_S01_EXPOTIME = 0x0A804;
            const uint16_t CFGCNT_S01_FRAMERATE = 0x0A805;
            const uint16_t CFGCNT_S01_PS = 0x0A806;
            const uint16_t CFGCNT_S01_PLLSET = 0x0A807;
            const uint16_t CFGCNT_S02_EXPOTIME = 0x0A808;
            const uint16_t CFGCNT_S02_FRAMERATE = 0x0A809;
            const uint16_t CFGCNT_S02_PS = 0x0A80A;
            const uint16_t CFGCNT_S02_PLLSET = 0x0A80B;
            const uint16_t CFGCNT_S03_EXPOTIME = 0x0A80C;
            const uint16_t CFGCNT_S03_FRAMERATE = 0x0A80D;
            const uint16_t CFGCNT_S03_PS = 0x0A80E;
            const uint16_t CFGCNT_S03_PLLSET = 0x0A80F;
            const uint16_t CFGCNT_S04_EXPOTIME = 0x0A810;
            const uint16_t CFGCNT_S04_FRAMERATE = 0x0A811;
            const uint16_t CFGCNT_S04_PS = 0x0A812;
            const uint16_t CFGCNT_S04_PLLSET = 0x0A813;
            const uint16_t CFGCNT_S05_EXPOTIME = 0x0A814;
            const uint16_t CFGCNT_S05_FRAMERATE = 0x0A815;
            const uint16_t CFGCNT_S05_PS = 0x0A816;
            const uint16_t CFGCNT_S05_PLLSET = 0x0A817;
            const uint16_t CFGCNT_S06_EXPOTIME = 0x0A818;
            const uint16_t CFGCNT_S06_FRAMERATE = 0x0A819;
            const uint16_t CFGCNT_S06_PS = 0x0A81A;
            const uint16_t CFGCNT_S06_PLLSET = 0x0A81B;
            const uint16_t CFGCNT_S07_EXPOTIME = 0x0A81C;
            const uint16_t CFGCNT_S07_FRAMERATE = 0x0A81D;
            const uint16_t CFGCNT_S07_PS = 0x0A81E;
            const uint16_t CFGCNT_S07_PLLSET = 0x0A81F;
            const uint16_t CFGCNT_S08_EXPOTIME = 0x0A820;
            const uint16_t CFGCNT_S08_FRAMERATE = 0x0A821;
            const uint16_t CFGCNT_S08_PS = 0x0A822;
            const uint16_t CFGCNT_S08_PLLSET = 0x0A823;
            const uint16_t CFGCNT_S09_EXPOTIME = 0x0A824;
            const uint16_t CFGCNT_S09_FRAMERATE = 0x0A825;
            const uint16_t CFGCNT_S09_PS = 0x0A826;
            const uint16_t CFGCNT_S09_PLLSET = 0x0A827;
            const uint16_t CFGCNT_S10_EXPOTIME = 0x0A828;
            const uint16_t CFGCNT_S10_FRAMERATE = 0x0A829;
            const uint16_t CFGCNT_S10_PS = 0x0A82A;
            const uint16_t CFGCNT_S10_PLLSET = 0x0A82B;
            const uint16_t CFGCNT_S11_EXPOTIME = 0x0A82C;
            const uint16_t CFGCNT_S11_FRAMERATE = 0x0A82D;
            const uint16_t CFGCNT_S11_PS = 0x0A82E;
            const uint16_t CFGCNT_S11_PLLSET = 0x0A82F;
            const uint16_t CFGCNT_S12_EXPOTIME = 0x0A830;
            const uint16_t CFGCNT_S12_FRAMERATE = 0x0A831;
            const uint16_t CFGCNT_S12_PS = 0x0A832;
            const uint16_t CFGCNT_S12_PLLSET = 0x0A833;
            const uint16_t CFGCNT_S13_EXPOTIME = 0x0A834;
            const uint16_t CFGCNT_S13_FRAMERATE = 0x0A835;
            const uint16_t CFGCNT_S13_PS = 0x0A836;
            const uint16_t CFGCNT_S13_PLLSET = 0x0A837;
            const uint16_t CFGCNT_S14_EXPOTIME = 0x0A838;
            const uint16_t CFGCNT_S14_FRAMERATE = 0x0A839;
            const uint16_t CFGCNT_S14_PS = 0x0A83A;
            const uint16_t CFGCNT_S14_PLLSET = 0x0A83B;
            const uint16_t CFGCNT_S15_EXPOTIME = 0x0A83C;
            const uint16_t CFGCNT_S15_FRAMERATE = 0x0A83D;
            const uint16_t CFGCNT_S15_PS = 0x0A83E;
            const uint16_t CFGCNT_S15_PLLSET = 0x0A83F;
            const uint16_t CFGCNT_S16_EXPOTIME = 0x0A840;
            const uint16_t CFGCNT_S16_FRAMERATE = 0x0A841;
            const uint16_t CFGCNT_S16_PS = 0x0A842;
            const uint16_t CFGCNT_S16_PLLSET = 0x0A843;
            const uint16_t CFGCNT_S17_EXPOTIME = 0x0A844;
            const uint16_t CFGCNT_S17_FRAMERATE = 0x0A845;
            const uint16_t CFGCNT_S17_PS = 0x0A846;
            const uint16_t CFGCNT_S17_PLLSET = 0x0A847;
            const uint16_t CFGCNT_S18_EXPOTIME = 0x0A848;
            const uint16_t CFGCNT_S18_FRAMERATE = 0x0A849;
            const uint16_t CFGCNT_S18_PS = 0x0A84A;
            const uint16_t CFGCNT_S18_PLLSET = 0x0A84B;
            const uint16_t CFGCNT_S19_EXPOTIME = 0x0A84C;
            const uint16_t CFGCNT_S19_FRAMERATE = 0x0A84D;
            const uint16_t CFGCNT_S19_PS = 0x0A84E;
            const uint16_t CFGCNT_S19_PLLSET = 0x0A84F;
            const uint16_t CFGCNT_S20_EXPOTIME = 0x0A850;
            const uint16_t CFGCNT_S20_FRAMERATE = 0x0A851;
            const uint16_t CFGCNT_S20_PS = 0x0A852;
            const uint16_t CFGCNT_S20_PLLSET = 0x0A853;
            const uint16_t CFGCNT_S21_EXPOTIME = 0x0A854;
            const uint16_t CFGCNT_S21_FRAMERATE = 0x0A855;
            const uint16_t CFGCNT_S21_PS = 0x0A856;
            const uint16_t CFGCNT_S21_PLLSET = 0x0A857;
            const uint16_t CFGCNT_S22_EXPOTIME = 0x0A858;
            const uint16_t CFGCNT_S22_FRAMERATE = 0x0A859;
            const uint16_t CFGCNT_S22_PS = 0x0A85A;
            const uint16_t CFGCNT_S22_PLLSET = 0x0A85B;
            const uint16_t CFGCNT_S23_EXPOTIME = 0x0A85C;
            const uint16_t CFGCNT_S23_FRAMERATE = 0x0A85D;
            const uint16_t CFGCNT_S23_PS = 0x0A85E;
            const uint16_t CFGCNT_S23_PLLSET = 0x0A85F;
            const uint16_t CFGCNT_S24_EXPOTIME = 0x0A860;
            const uint16_t CFGCNT_S24_FRAMERATE = 0x0A861;
            const uint16_t CFGCNT_S24_PS = 0x0A862;
            const uint16_t CFGCNT_S24_PLLSET = 0x0A863;
            const uint16_t CFGCNT_S25_EXPOTIME = 0x0A864;
            const uint16_t CFGCNT_S25_FRAMERATE = 0x0A865;
            const uint16_t CFGCNT_S25_PS = 0x0A866;
            const uint16_t CFGCNT_S25_PLLSET = 0x0A867;
            const uint16_t CFGCNT_S26_EXPOTIME = 0x0A868;
            const uint16_t CFGCNT_S26_FRAMERATE = 0x0A869;
            const uint16_t CFGCNT_S26_PS = 0x0A86A;
            const uint16_t CFGCNT_S26_PLLSET = 0x0A86B;
            const uint16_t CFGCNT_S27_EXPOTIME = 0x0A86C;
            const uint16_t CFGCNT_S27_FRAMERATE = 0x0A86D;
            const uint16_t CFGCNT_S27_PS = 0x0A86E;
            const uint16_t CFGCNT_S27_PLLSET = 0x0A86F;
            const uint16_t CFGCNT_S28_EXPOTIME = 0x0A870;
            const uint16_t CFGCNT_S28_FRAMERATE = 0x0A871;
            const uint16_t CFGCNT_S28_PS = 0x0A872;
            const uint16_t CFGCNT_S28_PLLSET = 0x0A873;
            const uint16_t CFGCNT_S29_EXPOTIME = 0x0A874;
            const uint16_t CFGCNT_S29_FRAMERATE = 0x0A875;
            const uint16_t CFGCNT_S29_PS = 0x0A876;
            const uint16_t CFGCNT_S29_PLLSET = 0x0A877;
            const uint16_t CFGCNT_S30_EXPOTIME = 0x0A878;
            const uint16_t CFGCNT_S30_FRAMERATE = 0x0A879;
            const uint16_t CFGCNT_S30_PS = 0x0A87A;
            const uint16_t CFGCNT_S30_PLLSET = 0x0A87B;
            const uint16_t CFGCNT_S31_EXPOTIME = 0x0A87C;
            const uint16_t CFGCNT_S31_FRAMERATE = 0x0A87D;
            const uint16_t CFGCNT_S31_PS = 0x0A87E;
            const uint16_t CFGCNT_S31_PLLSET = 0x0A87F;
            const uint16_t CFGCNT_TRIG = 0x0A880;
            const uint16_t CFGCNT_STATUS = 0x0A881;
            const uint16_t CFGCNT_CSICFG = 0x0A882;
            const uint16_t CFGCNT_PIFCCFG = 0x0A883;
            const uint16_t CFGCNT_PIFTCFG = 0x0A884;
            const uint16_t CFGCNT_BINCFG = 0x0A885;
            const uint16_t CFGCNT_ROICMINREG = 0x0A886;
            const uint16_t CFGCNT_ROICMAXREG = 0x0A887;
            const uint16_t CFGCNT_ROIRMINREG = 0x0A888;
            const uint16_t CFGCNT_ROIRMAXREG = 0x0A889;
            const uint16_t CFGCNT_ROS1 = 0x0A88A;
            const uint16_t CFGCNT_ROS2 = 0x0A88B;
            const uint16_t CFGCNT_IFDEL = 0x0A88C;
            const uint16_t CFGCNT_CTRLSEQ = 0x0A88D;
            const uint16_t CFGCNT_EXPCFG1 = 0x0A88E;
            const uint16_t CFGCNT_EXPCFG2 = 0x0A88F;
            const uint16_t CFGCNT_EXPCFG3 = 0x0A890;
            const uint16_t CFGCNT_EXPCFG4 = 0x0A891;
            const uint16_t CFGCNT_PSOUT = 0x0A892;
            const uint16_t CFGCNT_PLLCFG1_LUT1 = 0x0A893;
            const uint16_t CFGCNT_PLLCFG2_LUT1 = 0x0A894;
            const uint16_t CFGCNT_PLLCFG3_LUT1 = 0x0A895;
            const uint16_t CFGCNT_PLLCFG1_LUT2 = 0x0A896;
            const uint16_t CFGCNT_PLLCFG2_LUT2 = 0x0A897;
            const uint16_t CFGCNT_PLLCFG3_LUT2 = 0x0A898;
            const uint16_t CFGCNT_PLLCFG1_LUT3 = 0x0A899;
            const uint16_t CFGCNT_PLLCFG2_LUT3 = 0x0A89A;
            const uint16_t CFGCNT_PLLCFG3_LUT3 = 0x0A89B;
            const uint16_t CFGCNT_PLLCFG1_LUT4 = 0x0A89C;
            const uint16_t CFGCNT_PLLCFG2_LUT4 = 0x0A89D;
            const uint16_t CFGCNT_PLLCFG3_LUT4 = 0x0A89E;
            const uint16_t CSI2_CSIFCTRL = 0x08800;
            const uint16_t CSI2_CSIHSIZE = 0x08801;
            const uint16_t CSI2_CSIVSIZE = 0x08802;
            const uint16_t CSI2_CSICFG = 0x08803;
            const uint16_t CSI2_CSIDBADDR = 0x08805;
            const uint16_t CSI2_CSIPDBADDR = 0x08806;
            const uint16_t CSI2_CLCFG = 0x08807;
            const uint16_t CSI2_SPARE = 0x08808;
            const uint16_t iSMHWS_DMACTRL = 0x09000;
            const uint16_t iSMHWS_DMACFG = 0x09001;
            const uint16_t iSMHWS_DMASOURCE = 0x09002;
            const uint16_t iSMHWS_DMADESTIN = 0x09003;
            const uint16_t iSMHWS_DMACOUNT = 0x09004;
            const uint16_t iSMHWS_ICUBANKSEL = 0x09006;
            const uint16_t iSMHWS_ICUPOLARITY = 0x09007;
            const uint16_t iSMHWS_ICUEDGE = 0x09008;
            const uint16_t iSMHWS_ICUSRCSEL = 0x09009;
            const uint16_t iSMHWS_ICUMASKSET = 0x0900A;
            const uint16_t iSMHWS_ICUMASKCLEAR = 0x0900B;
            const uint16_t iSMHWS_ICUIRQENSET = 0x0900C;
            const uint16_t iSMHWS_ICUIRQENCLEAR = 0x0900D;
            const uint16_t iSMHWS_ICUIRQACK = 0x0900E;
            const uint16_t iSMHWS_ICUIRQPRIO1 = 0x09011;
            const uint16_t iSMHWS_ICUIRQPRIO2 = 0x09012;
            const uint16_t iSMHWS_ICUIRQPRIO3 = 0x09013;
            const uint16_t iSMHWS_ICUIRQPRIO4 = 0x09014;
            const uint16_t iSMHWS_TMR1MODE = 0x09015;
            const uint16_t iSMHWS_TMR1PREVAL = 0x09016;
            const uint16_t iSMHWS_TMR2MODE = 0x09018;
            const uint16_t iSMHWS_TMR2PREVAL = 0x09019;
            const uint16_t iSMHWS_SPARE = 0x0901C;
            const uint16_t iSM_EN = 0x0C400;
            const uint16_t iSM_CTRL = 0x0C401;
            const uint16_t iSM_MEMPAGE = 0x0C402;
            const uint16_t iSM_IADDR = 0x0C403;
            const uint16_t iSM_IDATA = 0x0C404;
            const uint16_t iSM_DIVT = 0x0C405;
            const uint16_t iSM_DIVT0_ADR = 0x0C406;
            const uint16_t iSM_DIVT1_ADR = 0x0C407;
            const uint16_t iSM_DIVT2_ADR = 0x0C408;
            const uint16_t iSM_DIVT3_ADR = 0x0C409;
            const uint16_t iSM_DIVT4_ADR = 0x0C40A;
            const uint16_t iSM_DIVTIRQSRC = 0x0C40B;
            const uint16_t iSM_GPO = 0x0C40C;
            const uint16_t iSM_FUNCNUM = 0x0C40D;
            const uint16_t iSM_TESTRES = 0x0C40E;
            const uint16_t iSM_ISMSTATE = 0x0C40F;
            const uint16_t iSM_HDU_CTRL = 0x0C410;
            const uint16_t iSM_HDU_BRKPNT_0 = 0x0C411;
            const uint16_t iSM_HDU_BRKPNT_1 = 0x0C412;
            const uint16_t iSM_HDU_BRKPNT_2 = 0x0C413;
            const uint16_t iSM_HDU_BRKPNT_3 = 0x0C414;
            const uint16_t iSM_SPARE = 0x0C416;
            const uint16_t MTCU_EN = 0x09800;
            const uint16_t MTCU_TRIG = 0x09801;
            const uint16_t MTCU_STATUS = 0x09802;
            const uint16_t MTCU_ROICMINREG = 0x09803;
            const uint16_t MTCU_ROICMAXREG = 0x09804;
            const uint16_t MTCU_ROIRMINREG = 0x09805;
            const uint16_t MTCU_ROIRMAXREG = 0x09806;
            const uint16_t MTCU_ROS1 = 0x09807;
            const uint16_t MTCU_ROS2 = 0x09808;
            const uint16_t MTCU_IFDEL = 0x09809;
            const uint16_t MTCU_CTRLSEQ = 0x0980A;
            const uint16_t MTCU_EXPCFG1 = 0x0980B;
            const uint16_t MTCU_EXPCFG2 = 0x0980C;
            const uint16_t MTCU_EXPCFG3 = 0x0980D;
            const uint16_t MTCU_EXPCFG4 = 0x0980E;
            const uint16_t MTCU_PSOUT = 0x0980F;
            const uint16_t MTCU_EXPOTIME = 0x09810;
            const uint16_t MTCU_FRAMERATE = 0x09811;
            const uint16_t MTCU_PS = 0x09812;
            const uint16_t MTCU_POWERCTRL = 0x09813;
            const uint16_t MTCU_PLLTIMER = 0x09814;
            const uint16_t MTCU_PLLDELAYTIMER = 0x09815;
            const uint16_t MTCU_TEST = 0x0981A;
            const uint16_t MTCU_SPARE = 0x0981B;
            const uint16_t PIF_PIFFCTRL = 0x08000;
            const uint16_t PIF_PIFHSIZE = 0x08001;
            const uint16_t PIF_PIFVSIZE = 0x08002;
            const uint16_t PIF_PIFCCFG = 0x08003;
            const uint16_t PIF_PIFTCFG = 0x08004;
            const uint16_t PIF_PIFDBADDR = 0x08005;
            const uint16_t PIF_PIFPDBADDR = 0x08006;
            const uint16_t PIF_PSEUDODATA1 = 0x08008;
            const uint16_t PIF_PSEUDODATA2 = 0x08009;
            const uint16_t PIF_PSEUDODATA3 = 0x0800A;
            const uint16_t PIF_PSEUDODATA4 = 0x0800B;
            const uint16_t PIF_SPARE = 0x0800C;

            const uint16_t MTCU_SEQNUM = 0x9819;
            const uint16_t DPRAM_FRAMECNT = 0x0170;

            const uint16_t iSM_FWSTARTADDRESS = 0xb800; //Start of the firmware program address space (page1 or page2)
            const uint16_t iSM_FW_RAM_VERSION_MSB = 0xbff8; //Location of the RAM firmware revision (at page1)
            const uint16_t iSM_FW_RAM_VERSION_LSB = 0xbff9; //Location of the RAM firmware revision (at page1)

            const uint16_t RECONFIG_COUNTER = 516;
        }
    }
}
