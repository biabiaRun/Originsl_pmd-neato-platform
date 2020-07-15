/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <imager/ImagerM2450.hpp>
#include <imager/M2450/ImagerConstants.hpp>
#include <imager/M2450_A11/ImagerRegisters.hpp>
#include <imager/M2450_A11/ImagerBaseConfig.hpp>
#include <imager/ModPllStrategyM2450_A12.hpp>

#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <set>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::imager::M2450;

const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > ImagerM2450::PSMAPPING_CLK4 =
{
    {
        ImagerRawFrame::ImagerDutyCycle::DC_0,
        {
            { 0, 0 },
            { 90, 0 },
            { 180, 0 },
            { 270, 0 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_25,
        {
            { 0, 0 << 5 | 6 << 1 },
            { 90, 2 << 5 | 0 << 1 },
            { 180, 4 << 5 | 2 << 1 },
            { 270, 6 << 5 | 4 << 1 }
        }
    },
    {
        //note: this is a trailing dutycycle, normally not used any more,
        //      but as all picoFlexx are calibrated using this it will not be changed
        ImagerRawFrame::ImagerDutyCycle::DC_25_DEPRECATED,
        {
            { 0, 0 << 5 | 2 << 1 },
            { 90, 2 << 5 | 4 << 1 },
            { 180, 4 << 5 | 6 << 1 },
            { 270, 6 << 5 | 0 << 1 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_37_5,
        {
            { 0, 10 },
            { 90, 78 },
            { 180, 130 },
            { 270, 198 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_37_5_DEPRECATED,
        {
            { 0, 0 << 5 | 3 << 1 },
            { 90, 2 << 5 | 5 << 1 },
            { 180, 4 << 5 | 7 << 1 },
            { 270, 6 << 5 | 1 << 1 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_50,
        {
            { 0,     8 | 0 << 9 },
            { 90,   76 | 2 << 9 },
            { 180, 128 | 4 << 9 },
            { 270, 196 | 6 << 9 }
        }
    },
};

ImagerM2450::ImagerM2450 (const ImagerParameters &params,
                          size_t measurementBlockCount,
                          size_t measurementBlockCapacity) :
    Imager (params, 4, 3, 4, measurementBlockCount, measurementBlockCapacity)
{
    if (params.illuminationPad != ImgIlluminationPad::SE_P &&
            params.illuminationPad != ImgIlluminationPad::LVDS)
    {
        throw InvalidValue ("The specified illumination pad is not supported");
    }

    if (params.illuminationPad == ImgIlluminationPad::LVDS &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_50)
    {
        throw InvalidValue ("The specified dutycycle is not supported for LVDS");
    }

    if (params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_0 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_25 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_25_DEPRECATED &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_37_5 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_37_5_DEPRECATED &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_50)
    {
        throw InvalidValue ("The specified dutycycle is not supported");
    }

    m_pllCalc = std::unique_ptr<ModPllStrategyM2450_A12> (new ModPllStrategyM2450_A12 (params.systemFrequency));

    m_rowLimitSensor = 288;
    m_columnLimitSensor = 352;
}

ImagerM2450::~ImagerM2450()
{

}

void ImagerM2450::initialize()
{
    Imager::initialize();

    //enqueue supported predefined MODPLL frequencies of base configuration
    m_lutAssignment[30000000] = 0;
    m_lutAssignment[60000000] = 1;
    m_lutAssignment[20000000] = 2;
    m_lutAssignment[99500000] = 3;
}

std::string ImagerM2450::getSerialNumber()
{
    std::string serialNr = ImagerM2450Serial::calculateSerialNumber (getSerialRegisters());
    logMessage (ImageSensorLogType::Comment, m_serial + " registered as " + serialNr);
    m_serial = serialNr;

    logMessage (ImageSensorLogType::Comment, "Serial number: " + m_serial);

    return m_serial;
}

ImagerVerificationStatus ImagerM2450::verifyModulationSettings (const ImagerUseCaseDefinition &useCase)
{
    return Imager::verifyModulationSettings (useCase, CLKDIV, MODPLLLUTCOUNT);
}

bool ImagerM2450::isValidExposureTime (bool mixedModeActive, size_t overallRawFrameCount, uint32_t exposureTime, uint32_t modFreq)
{
    if (ImagerRawFrame::MODFREQ_AUTO() == modFreq)
    {
        modFreq = getGrayscaleModulationFrequency();
    }

    //calculate exposure register value
    const double regExposure = std::floor (static_cast<double> (static_cast<double> (exposureTime)) *
                                           static_cast<double> (modFreq) / static_cast<double> (8.0 * 1.0e6));

    return regExposure <= 65535.0; //compare if value fits into expo register
}

/*
* This function calculates the exact time one raw frame takes during the whole processing.
* The function shows the different phases of the process and calculates the single values to sum them up at the end.
* The comments mean the cells in the frame rate calculation excel sheet, since the calculations here are based on this.
*/
bool ImagerM2450::calcRawFrameRateTime (
    const ImagerUseCaseDefinition &useCase,
    uint32_t expoTime,
    uint32_t modFreq,
    bool isFirstRawFrame,
    double &rawFrameTime) const
{
    uint16_t sizeColumn;
    uint16_t sizeRow;
    uint32_t modfreq = modFreq;
    useCase.getImage (sizeColumn, sizeRow);

    if (ImagerRawFrame::MODFREQ_AUTO() == modfreq)
    {
        modfreq = getGrayscaleModulationFrequency();
    }

    const double fFrameRate = useCase.getRawFrameRate();

    const double clk_illu_equ = FSYSCLK / static_cast<double> (modfreq);

    //==============================================
    // SequenceCfg Time
    //==============================================

    //The sequence configuration takes different time depending on if it is the first frame or not (pll locking different)
    //D76 / D156
    const double tSeqConfig = isFirstRawFrame ? sequenceCfgTime_1stFrame : sequenceCfgTime_regFrame;

    //==============================================
    // Exposure Time
    //==============================================

    //calculate exposure register value
    const auto exposureTime = static_cast<double> (expoTime);
    const double regExposure = std::floor (exposureTime * static_cast<double> (modfreq) / static_cast<double> (8.0f * 1.0e6));

    //Calculate the active exposure clock cycles. This consists of the configured exposure time + some additional delays like warmup, ...
    // J78
    double cycExposure = std::ceil ( (6 + exposurePreIlluCyc + exposureWarmupCyc + (8 * exposurePreModScaleCyc) + (8.0 * regExposure) + 1 + exposureRhDelayCyc) * clk_illu_equ + 3);

    //Add the exposure unit start cycles (J77)
    cycExposure += std::ceil (1 + 2 * clk_illu_equ + 1);

    //Calculate complete exposure time (D79)
    const double tExposure = cycExposure / FSYSCLK;

    //==============================================
    // Powerup
    //==============================================

    //Nothing to calculate, use constant powerUpTime

    //==============================================
    // Interface trigger and readout config
    //==============================================

    //Nothing to calculate, use constant ifTrigAndReadoutCfgTime

    //==============================================
    // Prepare for frame start
    //==============================================

    //Nothing to calculate, use constant prepareFrameStartTime

    //==============================================
    // Readout
    //==============================================

    //calculate effective tx pixel
    const bool hasDarkPixels = false;
    const double effectiveTxPixels =
        sizeColumn + static_cast<double> ( (hasDarkPixels ? 1. : 0.) * 16.0);

    //calculate effective rows (by default a pseudo data line is added)
    const double effectiveTxRows =
        1.0 + sizeRow + static_cast<double> ( (hasDarkPixels ? 1. : 0.) * 16.0);

    //get some delay values which can be different for different imagers (function implemented in sub class)
    double cyc_ss_lblank;
    double cycIfdelay;
    double cycAdcSocd;
    double cycAdcOddd;
    getReadoutDelays (cycIfdelay, cyc_ss_lblank, cycAdcSocd, cycAdcOddd);

    const double cyc_ss_dark = cycAdcSocd + cycAdcOddd; //J216 = F25 + F26
    const double cyc_ss_precharge = hasDarkPixels ? 2. : (cycAdcSocd + cycAdcOddd); //J217 = 2 or F25 + F26
    const double cyc_readout = hasDarkPixels ? 0. : (2. * cycAdcSocd) + cycAdcOddd; //J218 = 0 or (2 * F25) + F26
    const double cyc_dummy = (dummyConvCyc + 1) * (cycAdcSocd + cycAdcOddd);        //L179
    const double cyc_adc_conv = effectiveTxPixels / 16 * (cycAdcSocd + cycAdcOddd); //L210
    //The number of additional cycles that the first line takes longer (differences in J178+J179 vs. J202)
    const double cyc_diffFirstLine = cyc_dummy + cycAdcSocd + cycAdcOddd - cyc_ss_lblank;

    double cycInterface = 0.;

    const auto imagerDataTransferType = m_imagerParams.imageDataTransferType;

    //Formula in J210/211:
    switch (imagerDataTransferType)
    {
        case ImgImageDataTransferType::PIF:
            cycInterface = (std::ceil ( (2.0 * effectiveTxPixels + 1.0) / 11.0) + 1.0) * 11.0;
            break;
        case ImgImageDataTransferType::MIPI_1LANE:
            cycInterface = 77. + (effectiveTxPixels * 2.) + 52.; //only valid for disabled short packets
            break;
        case ImgImageDataTransferType::MIPI_2LANE:
            //J211       = 74  + J33               + 55   => MIPI, 2-Lane, disabled short packets
            cycInterface = 74. + effectiveTxPixels + 55.; //only valid for disabled short packets
            break;
        default:
            throw NotImplemented();
            break;
    }

    //L189:
    const double cyc_wait = std::ceil ( (cyc_ss_precharge + cyc_readout + cc_binstatCyc + binstatCyc + cycIfdelay + cc_iftrigCyc + cycInterface) / (cycAdcSocd + cycAdcOddd)) * (cycAdcSocd + cycAdcOddd) - (cycAdcSocd + cycAdcOddd + cyc_adc_conv);

    //L190:
    const double cycEndOnOddHandling = cycAdcSocd + cycAdcOddd; //assumes: endOnOdd setting fits cyc_wait

    //number of cycles of a "regular line with interface" => H111 / H134 / H191 / H214
    //The calculation is a little bit reduced and does not reflect the excel calculation one-to-one. But it produces the same result.
    const double cycRegLineWithInterface = cyc_ss_lblank + cyc_ss_dark + cyc_ss_precharge + cyc_adc_conv + cyc_wait + cycEndOnOddHandling;

    //n-lines readout cycles and time (binning assumed to be "1") => H145 / H225
    const double cycNLinesReadout = (effectiveTxRows - 1) * cycRegLineWithInterface + cyc_diffFirstLine;  //The first line takes more cycles so add the diff once
    const double tNLinesReadout = cycNLinesReadout / FSYSCLK;

    //==============================================
    // Shutdown and framerate
    //==============================================

    const double tShutDownAndFramerate = 50.0 / FSYSCLK;

    //==============================================
    // Sum up complete frame
    //==============================================

    //calculate raw frame time (assuming there is no frame rate delay counter active)
    //This is the sum of all the single phases before
    rawFrameTime = tSeqConfig + tExposure + powerUpTime + ifTrigAndReadoutCfgTime + prepareFrameStartTime + tNLinesReadout + tShutDownAndFramerate;

    bool rawFameRateFeasible = true;

    //if a fFrameRate is specified checks are required if the raw frame rate is feasible
    if (fFrameRate > 0.)
    {
        if (rawFrameTime <= 1. / fFrameRate)
        {
            //either the raw frame time fits perfectly or the raw frame delay counter can be set to a certain value;
            //therefore the raw frame time can be defined purely by the frame rate counter
            rawFrameTime = 1. / fFrameRate;
        }
        else
        {
            //the rawFrameTime will contain the time it requires, but it is not feasible using the specified raw frame rate
            rawFameRateFeasible = false;
        }
    }

    return rawFameRateFeasible;
}

const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > &ImagerM2450::getPhaseMapping()
{
    return PSMAPPING_CLK4;
}

uint16_t ImagerM2450::calcRegExposure (const uint32_t &expoTime, const uint32_t modfreq) const
{
    return static_cast<uint16_t> (std::floor (static_cast<double> (expoTime) *
                                  static_cast<double> (modfreq) / static_cast<double> (8.0 * 1.0e6)));
}

void ImagerM2450::adjustRowCount (uint16_t &row)
{
    // add one line because pseudodata line replaces the first line
    row++;
}

bool ImagerM2450::isFirstFrame (const std::vector<ImagerRawFrame> &rfList, size_t index) const
{
    if (index == 0)
    {
        return true;
    }

    if (rfList.at (index - 1).isEndOfLinkedMeasurement)
    {
        return true;
    }

    //all remaining cases
    return false;
}
