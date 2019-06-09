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
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/NarrowCast.hpp>
#include <common/StringFunctions.hpp>

#include <imager/ImagerM2452.hpp>
#include <imager/M2452/PseudoDataInterpreter.hpp>
#include <imager/M2452/ImagerRegisters.hpp>
#include <imager/ModPllStrategyM2452.hpp>
#include <imager/DphyPllStrategyM2452.hpp>
#include <imager/ISequenceGenerator.hpp>

#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <set>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::imager::M2452;

namespace
{
    class SequenceGeneratorM2452 : ISequenceGenerator
    {
    public:
        void generateMeasurementBlockConfig (const std::vector<MeasurementBlock> &mbList,
                                             std::map < uint16_t, uint16_t > &regChanges) override
        {
            const uint16_t SEQIDXOFFSET = 3u;
            uint16_t idx = 0u;

            for (const auto &mb : mbList)
            {
                for (size_t sidx = 0; sidx < mb.sequence.size(); sidx++)
                {
                    const auto &seq = mb.sequence.at (sidx);
                    regChanges[static_cast<uint16_t> (RECONFIG_CFGCNT_S00_EXPOTIME + idx)] = seq.expo;
                    regChanges[static_cast<uint16_t> (RECONFIG_CFGCNT_S00_PS + idx)] = static_cast<uint16_t> (seq.ps | seq.pllset << 9);
                    regChanges[static_cast<uint16_t> (RECONFIG_CFGCNT_S00_FRAMERATE + idx)] = seq.fr;

                    //move up to the next LUT assignment register address
                    idx = static_cast<uint16_t> (idx + SEQIDXOFFSET);
                }
            }

            regChanges[RECONFIG_FRAMERATE] = narrow_cast<uint16_t> (mbList.front().frameRateCounter);
            regChanges[CFGCNT_CTRLSEQ] = static_cast<uint16_t> (mbList.front().sequence.size() - 1);
        }
    };
}

const std::array<uint16_t, 4> ImagerM2452::EXPO_PRESCALE_MAP = { { 1, 8, 32, 128 } };

const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > ImagerM2452::PSMAPPING_CLK4 =
{
    {
        ImagerRawFrame::ImagerDutyCycle::DC_0,
        {
            { 0, 306 },
            { 90, 306 },
            { 180, 306 },
            { 270, 306 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_25,
        {
            { 0, 4 },
            { 90, 72 },
            { 180, 140 },
            { 270, 192 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_37_5,
        {
            { 0, 2 },
            { 90, 70 },
            { 180, 138 },
            { 270, 206 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_50,
        {
            { 0, 16 },
            { 90, 80 },
            { 180, 144 },
            { 270, 208 }
        }
    },
    {
        ImagerRawFrame::ImagerDutyCycle::DC_100,
        {
            { 0, 272 },
            { 90, 272 },
            { 180, 272 },
            { 270, 272 }
        }
    }

};

const std::string ImagerM2452::MSG_STARTLPFSMIDLE ("start lpfsm failed, lpfsm is idle");
const std::string ImagerM2452::MSG_DPHYPLLLOCK ("start lpfsm failed, DPHY PLL lock error");
const std::string ImagerM2452::MSG_ISMACTIVE ("start failed, ism is still active");
const std::string ImagerM2452::MSG_MODPLLLOCK ("start lpfsm failed, MOD PLL lock error");

ImagerM2452::ImagerM2452 (const ImagerParameters &params,
                          size_t measurementBlockCount,
                          size_t measurementBlockCapacity) :
    Imager (params, 4, 3, 4, measurementBlockCount, measurementBlockCapacity)
{
    if (params.imageDataTransferType != ImgImageDataTransferType::MIPI_1LANE &&
            params.imageDataTransferType != ImgImageDataTransferType::MIPI_2LANE)
    {
        throw InvalidValue ("The specified interface type is not supported");
    }

    if (!params.useSuperframe)
    {
        // The hardware can support individual frames, but as everything can be supported with
        // superframe mode this class doesn't support the CFGCNT_CSICFG settings for it.
        throw InvalidValue ("Only Superframe mode is supported");
    }

    if (params.illuminationPad != ImgIlluminationPad::SE_P)
    {
        throw InvalidValue ("The specified illumination pad is not supported");
    }

    if (params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_0 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_25 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_37_5 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_50 &&
            params.dutyCycle != ImagerRawFrame::ImagerDutyCycle::DC_100)
    {
        throw InvalidValue ("The specified dutycycle is not supported");
    }

    if (params.systemFrequency < 10000000 ||
            params.systemFrequency > 35000000 ||
            params.systemFrequency % 100000 != 0)
    {
        throw OutOfBounds ("the imager does not support camera modules with this system frequency");
    }

    if (params.externalTrigger != ImgTrigger::I2C)
    {
        throw InvalidValue ("The specified trigger mode is not supported");
    }

    m_pllCalc = std::unique_ptr<ModPllStrategyM2452> (new ModPllStrategyM2452 (params.systemFrequency));

    m_regTrigger = MTCU_LPFSMEN;
    m_valStartTrigger = 5u;
    m_valEndTrigger = 1u;
    m_regStatus = MTCU_STATUS;
    m_statusIdle = (1 << 1) | (1 << 0); // LPFSM idle and MTCU idle
    m_regReconfCnt = RECONFIG_COUNTER;

    m_regPllLutLower = RECONFIG_CFGCNT_PLLCFG1_LUT1;

    m_rowLimitSensor = 172;
    m_columnLimitSensor = 224;
}

ImagerM2452::~ImagerM2452()
{

}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2452::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new PseudoDataInterpreter);
    return pseudoDataInter;
}

std::string ImagerM2452::getSerialNumber()
{
    //read efuses and create an unique string that identifies the sensor
    std::vector < uint16_t > vals = getSerialRegisters();

    std::string serialNr = ImagerM2452Serial (vals.at (0), vals.at (1), vals.at (2), vals.at (3)).serialNumber;
    logMessage (ImageSensorLogType::Comment, m_serial + " registered as " + serialNr);
    m_serial = serialNr;

    logMessage (ImageSensorLogType::Comment, "Serial number: " + m_serial);

    return m_serial;
}

void ImagerM2452::downloadInitialConfig (const std::map < uint16_t, uint16_t > &baseConfig)
{
    //dphy pll configuration if system frequency deviates from default
    const auto systemFrequency = m_imagerParams.systemFrequency;
    if (systemFrequency != 24000000)
    {
        DphyPllStrategyM2452 pllCalc (m_imagerParams.systemFrequency);
        std::vector<uint16_t> pllcfg (8);

        if (!pllCalc.pllSettings (FSYSCLK * 3.0, false, pllcfg))
        {
            throw Exception ("Invalid dphy pll frequency specified");
        }

        logMessage (ImageSensorLogType::Comment, "Reconfiguration of DPHY PLL to " + toStdString (systemFrequency) + "Hz");

        writeImagerBurst (ANAIP_DPHYPLLCFG1, pllcfg);
    }

    initialRegisterConfig (baseConfig);

    uint16_t ifdelValue = static_cast<uint16_t> (m_imagerParams.interfaceDelay * FSYSCLK);

    ifdelValue = static_cast<uint16_t> (ifdelValue << 4); //shift up to right position (lower bits are the frame delay value)

    if (m_regDownloaded.count (CFGCNT_ROS))
    {
        //override a possible base config setting - module config setting has higher prio
        ifdelValue = static_cast<uint16_t> (ifdelValue | (m_regDownloaded[CFGCNT_ROS] & 0xF));
    }

    const std::map< uint16_t, uint16_t > padconfig{ { ANAIP_PSPADCFG, 0x1513 } };
    switch (m_imagerParams.illuminationPad)
    {
        case ImgIlluminationPad::SE_P:
            trackAndWriteRegisters (padconfig); // enable SE_P pad, disable SE_N pad
            break;
        default:
            throw NotImplemented();
    }

    const std::map< uint16_t, uint16_t > ifdel{ { CFGCNT_ROS, ifdelValue } };
    trackAndWriteRegisters (ifdel);

    switch (m_imagerParams.imageDataTransferType)
    {
        case ImgImageDataTransferType::MIPI_1LANE:
            //1 Lane with Superframes, do not use short packets
            trackAndWriteRegisters ({ { CFGCNT_CSICFG, 0x0112 } });
            break;
        case ImgImageDataTransferType::MIPI_2LANE:
            //2 Lane with Superframes, do not use short packets
            trackAndWriteRegisters ({ { CFGCNT_CSICFG, 0x0192 } });
            break;
        default:
            throw NotImplemented();
            break;
    }
}

void ImagerM2452::startCapture ()
{
    Imager::startCapture();

    uint16_t regValue = 0;
    readImagerRegister (m_regStatus, regValue);

    if (! (regValue & 2))
    {
        logMessage (ImageSensorLogType::Comment, MSG_CAPTUREBUSY);
        throw LogicError (MSG_CAPTUREBUSY);
    }

    // The sequencer settings are in the RECONFIG registers, ensure that they're copied to the
    // CFGCNT registers.  The RECONFIG_TRIGGER is needed for the A11 if it's been reset,
    // but is not needed for the first startup after power-on.
    readImagerRegister (ISM_ISMSTATE, regValue);
    writeImagerRegister (ISM_ISMSTATE, static_cast<uint16_t> (regValue | 0x0008)); // <-- activate safe reconfiguration firmware
    writeImagerRegister (RECONFIG_TRIGGER, 1); // <-- tell the reconfig FW to fetch the new config
    writeImagerRegister (ISM_DIVT0_ADR, 7); // <-- always jump into ROM before RAM
    writeImagerRegister (ISM_EN, 0x0001); // <-- start safe reconfig FW (to do a first copy of the config into the CFGCNT)

    m_bridge->sleepFor (std::chrono::microseconds (100));
    readImagerRegister (ISM_ISMSTATE, regValue);

    if (regValue & 1 << 14)
    {
        throw RuntimeError (MSG_ISMACTIVE);
    }

    //starting the sequencer
    m_executingUcd = m_preparedUcd;

    writeImagerRegister (m_regTrigger, m_valStartTrigger);

    // Wait for the firmware to update the status register
    // \todo ROYAL-2901 How long should this sleep be?
    m_bridge->sleepFor (std::chrono::microseconds (100));

    evaluatePostStartState();
}

void ImagerM2452::evaluatePostStartState()
{
    uint16_t regValue = 0;
    readImagerRegister (m_regStatus, regValue);

    if (regValue & 1 << 5)
    {
        logMessage (ImageSensorLogType::Comment, MSG_DPHYPLLLOCK);
        throw RuntimeError (MSG_DPHYPLLLOCK);
    }

    if (regValue & 1 << 4)
    {
        logMessage (ImageSensorLogType::Comment, MSG_MODPLLLOCK);
        throw RuntimeError (MSG_MODPLLLOCK);
    }

    if (regValue & 1 << 1)
    {
        logMessage (ImageSensorLogType::Comment, MSG_STARTLPFSMIDLE);
        throw RuntimeError (MSG_STARTLPFSMIDLE);
    }

    m_imagerState = ImagerState::Capturing;
}

void ImagerM2452::stopCapture()
{
    Imager::stopCapture();

    uint16_t regValue = 0;

    readImagerRegister (m_regStatus, regValue);

    if (regValue & 2)
    {
        logMessage (ImageSensorLogType::Comment, MSG_STOPMTCUIDLE);
        throw LogicError (MSG_STOPMTCUIDLE);
    }

    shutDownSequencer();

    m_imagerState = ImagerState::Ready;
}

void ImagerM2452::reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex)
{
    Imager::reconfigure (useCase, reconfigIndex);

    ImagerVerificationStatus status = verifyUseCase (useCase);

    if (ImagerVerificationStatus::SUCCESS == status)
    {
        //this method is most probably accessed directly by processing from a different thread,
        //as the processing implementation is unknown this is a critical section
        std::unique_lock <std::mutex> lock (m_reconfigLock);

        //create map for staging the changes of the exposure time only
        const auto regChanges = prepareUseCase (useCase);

        trackAndWriteRegisters (resolveConfiguration (regChanges));

        m_executingUcd = useCase;

        //get index which marks the previous UCD
        m_bridge->readImagerRegister (m_regReconfCnt, reconfigIndex);

        //tell the reconfig FW to fetch the new config
        writeImagerRegister (RECONFIG_TRIGGER, 1);
    }
    else
    {
        throw RuntimeError ("reconfiguration failed, not succeeded to verify configuration");
    }
}

ImagerVerificationStatus ImagerM2452::verifyModulationSettings (const ImagerUseCaseDefinition &useCase)
{
    return Imager::verifyModulationSettings (useCase, CLKDIV, MODPLLLUTCOUNT);
}

ImagerVerificationStatus ImagerM2452::verifyFrameRateSettings (const ImagerUseCaseDefinition &useCase, bool rawFrameBasedFrameRate)
{
    const auto regFrameRate = ::round (static_cast<double> (m_imagerParams.systemFrequency /
                                       (128. * static_cast<double> (useCase.getTargetRate()))));
    try
    {
        narrow_cast<uint16_t> (regFrameRate);
    }
    catch (const OutOfBounds &)
    {
        return ImagerVerificationStatus::FRAMERATE;
    }

    return Imager::verifyFrameRateSettings (useCase, false);
}

bool ImagerM2452::isValidExposureTime (bool enabledMixedMode, size_t overallRawFrameCount, uint32_t exposureTime, uint32_t modFreq)
{
    if (ImagerRawFrame::MODFREQ_AUTO() == modFreq)
    {
        modFreq = getGrayscaleModulationFrequency();
    }

    //assume highest possible prescaler to be in use
    const auto maxPreScaler = EXPO_PRESCALE_MAP.back();

    //calculate exposure register value
    const double regExposure = std::floor (static_cast<double> (static_cast<double> (exposureTime)) *
                                           static_cast<double> (modFreq) / (static_cast<double> (maxPreScaler) * 1.0e6));

    return regExposure <= 16383.; //compare if value fits into expo register
}

bool ImagerM2452::calcRawFrameRateTime (
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
    const uint16_t calcExposure = calcRegExposure (expoTime, modfreq);
    const double regExposure = ( (double) (calcExposure & 0x3FFF)) * EXPO_PRESCALE_MAP[calcExposure >> 14];

    //sys_clk_mod_mul = FSYSCLK / modfreq
    const double sys_clk_mod_mul = FSYSCLK / static_cast<double> (modfreq);

    //cycExposure = 9 + CEILING(sys_clk_mod_mul*(15 + PREILLU(10) + WARMUP(30) + 8*PREMODSCALE(16) + regExposure + reset(0)) + 4)
    const double cycExposure = 9. + std::ceil (sys_clk_mod_mul * (180. + regExposure) + 4.);

    //cycPowerup => cc_pll_pwd, cc_adc_bg, adc_bg_delay, cc_adc_en, cc_pixif, pixif_delay
    const double cycPowerup = 3116.; //default settings are used

    //cycIfTrigger => cc_trig_fe, cc_romode
    const double cycIfTrigger = 12.; //default settings are used

    //cycPrepare => Fblank(8)
    const double cycPrepare = 140.;

    double cycLBlank;
    double cycIfdelay;
    double cycAdcSocd;
    double cycAdcOddd;
    getReadoutDelays (cycIfdelay, cycLBlank, cycAdcSocd, cycAdcOddd);

    //ifstat depends on dphy frequency and short packets enable (disabled by default)
    const bool shortPacketsEnable = false;
    const double lanes = m_imagerParams.imageDataTransferType == ImgImageDataTransferType::MIPI_1LANE ? 1. : 2.;

    const double cycAdcBunches = 14.;
    const double cycExtraEol = 1.;
    const double cycConvRegRow = (cycLBlank + cycAdcBunches + cycExtraEol + 1.);
    const double cycRegLineDuration = cycConvRegRow * (cycAdcSocd + cycAdcOddd);

    //These timings are defined by ROYAL-1315 MIPI compliance (dphy lane configuration registers)
    const double sotInterfaceTime = 3.2e-7;
    const double eotInterfaceTime = 2.0e-7;

    const double shortP_HS_interface_time = ( (32. / lanes) + 8.) * 1.25e-09;
    const double longP_HS_interface_time = ( ( (32. + 16. + (sizeColumn * 12.)) / lanes) + 8.) * 1.25e-09;
    const double mtcu2dphy_overhead = 2.05e-7;
    const double dphy2mtcu_overhead = (1.) / FSYSCLK;
    double dphy_line = sotInterfaceTime + longP_HS_interface_time + eotInterfaceTime;
    dphy_line += shortPacketsEnable ? 2. * (sotInterfaceTime + eotInterfaceTime + shortP_HS_interface_time + 1.6e-07) : 0.;

    const double cycIfReadout = std::ceil ( (mtcu2dphy_overhead + dphy2mtcu_overhead + dphy_line) / (1. / FSYSCLK));
    const double cycIfstat = (std::ceil (cycIfReadout / 11.) + 1.) * 11.;

    const double cycRegLineConst = 14.; //readout, cc_adcif_cfg, cc_iftrig

    const double cycRegLineIfDominant =
        std::ceil ( (cycRegLineConst + (cycIfdelay + 1.) + cycIfstat - cycRegLineDuration) / (cycAdcSocd + cycAdcOddd))
        * (cycAdcSocd + cycAdcOddd);

    const double cycRegLine = cycRegLineDuration > cycIfstat ? cycRegLineDuration :
                              (std::fmod (cycRegLineIfDominant, 22.) > 0. ?
                               cycRegLineIfDominant + (cycAdcSocd + cycAdcOddd) : cycRegLineIfDominant);

    const double cycLastLineAdc = 51. + cycRegLineConst + (cycIfdelay + 1.) + cycIfstat;
    const double cycLastIfLine = 111. + cycRegLineConst + (cycIfdelay + 1.) + cycIfstat;

    //n-lines readout time
    const double cycNLinesReadout = ( (sizeRow - 1.) * cycRegLine) + cycLastLineAdc + cycLastIfLine;
    const double tAllLines = cycNLinesReadout / FSYSCLK;
    const double tShutDownAndFramerate = (18.) / FSYSCLK;

    //Note: The first frame takes more time (pll locking) and this time further increases
    //      if the low-power mode is active. The concrete value for this time depends on the power
    //      control register settings which must not be changed.
    const double tSeqConfig = isFirstRawFrame ? 115e-6 : (44. / FSYSCLK);

    //tToFrameStart = (Sequencecfg + cycExposure + cycPowerup + cycIfTrigger + cycPrepare) / FSYSCLK
    const double tToFrameStart = tSeqConfig + ( (cycExposure + cycPowerup + cycIfTrigger + cycPrepare) / FSYSCLK);

    //set the output parameter
    rawFrameTime = tToFrameStart + tAllLines + tShutDownAndFramerate;

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

void ImagerM2452::getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const
{
    if (m_regDownloaded.count (CFGCNT_ROS))
    {
        ifdel = static_cast<double> ( (m_regDownloaded.at (CFGCNT_ROS) & 0x7FF0) >> 4);
    }
    else
    {
        ifdel = 0.0;
    }

    if (m_regDownloaded.count (MTCU_ROS2))
    {
        lblank = static_cast<double> (m_regDownloaded.at (MTCU_ROS2) & 0x3F);
        cycAdcSocd = 2. + (2. * static_cast<double> ( (m_regDownloaded.at (MTCU_ROS2) >> 8) & 3));
        cycAdcOddd = 14. + (2. * static_cast<double> ( (m_regDownloaded.at (MTCU_ROS2) >> 10) & 0xF));
    }
    else
    {
        lblank = 4.;
        cycAdcSocd = 2.;
        cycAdcOddd = 20.;
    }
}

const std::map < ImagerRawFrame::ImagerDutyCycle, std::map < uint16_t, uint16_t > > &ImagerM2452::getPhaseMapping()
{
    return PSMAPPING_CLK4;
}

uint16_t ImagerM2452::calcRegExposure (const uint32_t &expoTime, const uint32_t modfreq) const
{
    auto texpo = [ = ] (uint16_t preScaler)
    {
        return std::floor (static_cast<double> (expoTime) * static_cast<double> (modfreq) / static_cast<double> (1.0e6 * preScaler));
    };

    for (std::size_t preScaler = 0; preScaler < EXPO_PRESCALE_MAP.size(); preScaler++)
    {
        if (texpo (EXPO_PRESCALE_MAP[preScaler]) <= (double) 0x3FFF)
        {
            return static_cast<uint16_t> (static_cast<uint16_t> (texpo (EXPO_PRESCALE_MAP[preScaler])) | preScaler << 14);
        }
    }

    // \todo JIRA-3090 this throw is unreachable, because the time was already checked in isValidExposureTime
    throw OutOfBounds ("Exposure time can not be represented to the imager");
}

std::map < uint16_t, uint16_t > ImagerM2452::prepareUseCase (const ImagerUseCaseDefinition &useCase)
{
    //create map for staging all changes
    std::map < uint16_t, uint16_t > regChanges;
    prepareFrameRateSettings (useCase);
    prepareModulationSettings (useCase, regChanges);
    preparePsSettings (useCase);
    prepareExposureSettings (useCase);

    SequenceGeneratorM2452().generateMeasurementBlockConfig (m_mbList, regChanges);

    uint16_t roiCMin, roiCMax, roiRMin, roiRMax;
    prepareROI (useCase, roiCMin, roiCMax, roiRMin, roiRMax);
    regChanges[CFGCNT_ROICOL] = static_cast<uint16_t> ( (roiCMax << 8) | roiCMin);
    regChanges[CFGCNT_ROIROW] = static_cast<uint16_t> ( (roiRMax << 8) | roiRMin);

    return regChanges;
}

void ImagerM2452::prepareMeasurementBlockTargetTime (MeasurementBlockId mbId, const double mbTargetTime, const double mbMeasurementTime)
{
    if (mbTargetTime > 0.)
    {
        m_mbList[mbId].sequence.back().fr = 0u;
        m_mbList[mbId].frameRateCounter =
            narrow_cast<uint32_t> (::round (mbTargetTime * static_cast<double> (m_imagerParams.systemFrequency) / 128.0));
    }
    else
    {
        m_mbList[mbId].frameRateCounter = 0u;
    }
}

std::vector < uint16_t > ImagerM2452::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

void ImagerM2452::adjustRowCount (uint16_t &row)
{
    // NoOp: pseudodata line is added to the data
}

