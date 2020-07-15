/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

/*
* This file defines all the constants to calculate timings of the imager
* The comments mean the cells in the frame rate calculation excel sheet, since the calculations, using the constants defined here, are based on this.
*/

#pragma once

namespace royale
{
    namespace imager
    {
        namespace M2450
        {
            // Sequence Config
            const double sequenceCfgTime_1stFrame = 1.158E-05;  //D76
            const double sequenceCfgTime_regFrame = 2.025E-07;  //D156

            // Exposure
            const uint16_t exposurePreIlluCyc = 10;     //F36
            const uint16_t exposureWarmupCyc = 30;      //F37
            const uint16_t exposurePreModScaleCyc = 16; //B39
            const uint16_t exposureRhDelayCyc = 5;      //F39

            //Readout
            const uint16_t cc_binstatCyc = 10;  //J126 / J206
            const uint16_t binstatCyc = 1;      //J127 / J207
            const uint16_t cc_iftrigCyc = 6;    //J129 / J209
            const uint16_t dummyConvCyc = 6;    //F27

            //Further time constants (changing when FSYSCLK changes)
            const double powerUpTime = 2.331E-05;           //D85 / D165
            const double ifTrigAndReadoutCfgTime = 9E-08;   //D88 / D168
            const double prepareFrameStartTime = 1.155E-06; //D97 / D177
        }
    }
}