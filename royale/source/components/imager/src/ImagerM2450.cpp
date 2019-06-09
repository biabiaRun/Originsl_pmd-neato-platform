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

    //calculate exposure register value
    const auto exposureTime = static_cast<double> (expoTime);
    const double regExposure = std::floor (exposureTime * static_cast<double> (modfreq) / static_cast<double> (8.0f * 1.0e6));

    //clk_illu_equ = (1 / rawFrameSet.modulationFrequency) / (1 / FSYSCLK)
    const double clk_illu_equ = FSYSCLK / static_cast<double> (modfreq);

    //cycActiveExposure =
    //  CEILING((6 + PREILLU(10) + WARMUP(30) + 8 * PREMODSCALE(16) + 8 * regExposure + 1 + RHDELAY(5))*clk_illu_equ + 3; 1)
    const double cycActiveExposure = std::ceil ( (46.0 + (128.0) + (8.0 * regExposure) + 6.0) * clk_illu_equ + 3.0);

    //tExposure = (cycexpo-up + cycActiveExpo) / fSys = (11 + cycActiveExposure) / fSys
    const double tExposure = (11.0 + cycActiveExposure) / (FSYSCLK);

    //some constants that change only when interface/SYSCLK changes, lets summarize them:
    // tToFrameStart = tSequenceConfig + tExposure + tPowerUp + tIfTriggerAndReadout + tPrepareForFrameStart
    //note: the first frame takes more time (pll locking)
    const double tSeqConfig = isFirstRawFrame ? 1.15725e-05 : 2.025e-07;
    const double tToFrameStart = static_cast<double> (tSeqConfig + tExposure + 2.331e-05 + 9e-08 + 1.155e-06);

    //calculate effective tx pixel
    const bool hasDarkPixels = false;
    const double effectiveTxPixels =
        sizeColumn + static_cast<double> ( (hasDarkPixels ? 1. : 0.) * 16.0);

    //calculate effective rows (by default a pseudo data line is added)
    const double effectiveTxRows =
        1.0 + sizeRow + static_cast<double> ( (hasDarkPixels ? 1. : 0.) * 16.0);

    double cyc_ss_lblank;
    double cycIfdelay;
    double cycAdcSocd;
    double cycAdcOddd;
    getReadoutDelays (cycIfdelay, cyc_ss_lblank, cycAdcSocd, cycAdcOddd);

    const double cyc_ss_dark = cycAdcSocd + cycAdcOddd;
    const double cyc_ss_precharge = hasDarkPixels ? 2. : (cycAdcSocd + cycAdcOddd);
    const double cyc_readout = hasDarkPixels ? 0. : (2. * cycAdcSocd) + cycAdcOddd;
    const double cycRegLineConst = cyc_ss_lblank + cyc_ss_dark + cyc_ss_precharge + cyc_readout + cycIfdelay + 17.;

    double cycInterface = 0.;

    const auto imagerDataTransferType = m_imagerParams.imageDataTransferType;

    switch (imagerDataTransferType)
    {
        case ImgImageDataTransferType::PIF:
            cycInterface = (std::ceil ( (2.0 * effectiveTxPixels + 1.0) / 11.0) + 1.0) * 11.0;
            break;
        case ImgImageDataTransferType::MIPI_1LANE:
            cycInterface = 77. + (effectiveTxPixels * 2.) + 52.; //only valid for disabled short packets
            break;
        case ImgImageDataTransferType::MIPI_2LANE:
            cycInterface = 74. + effectiveTxPixels + 55.; //only valid for disabled short packets
            break;
        default:
            throw NotImplemented();
            break;
    }

    //number of cycles of a "regular line with interface"
    const double cycRegLineWithInterface = (cycRegLineConst + cycInterface) + 3.;

    //n-lines readout time (note: simplified, normally first line would take a few cycles more)
    const double cycNLinesReadout = (cycRegLineWithInterface + (effectiveTxRows - 1.0) * (cycRegLineWithInterface));
    const double tNLinesReadout = cycNLinesReadout / FSYSCLK;
    const double tShutDownAndFramerate = 50.0 / FSYSCLK;

    //calculate raw frame time (assuming there is no frame rate delay counter active)
    rawFrameTime = tToFrameStart + tNLinesReadout + tShutDownAndFramerate;

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
