/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/Exception.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/OutOfBounds.hpp>

#include <imager/ImagerM2450_A11.hpp>
#include <imager/M2450_A11/ImagerRegisters.hpp>
#include <imager/M2450_A11/ImagerBaseConfig.hpp>
#include <imager/ModPllStrategyM2450_A12.hpp>
#include <imager/ISequenceGenerator.hpp>
#include <imager/M2450_A11/PseudoDataInterpreter.hpp>

#include <chrono>
#include <thread>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::imager::M2450_A11;

namespace
{
    class SequenceGeneratorM2450_A11 : ISequenceGenerator
    {
    public:
        void generateMeasurementBlockConfig (const std::vector<MeasurementBlock> &mbList,
                                             std::map < uint16_t, uint16_t > &regChanges) override
        {
            const uint16_t SEQIDXOFFSET = 4u;
            uint16_t idx = 0u;

            for (const auto &mb : mbList)
            {
                for (size_t sidx = 0; sidx < mb.sequence.size(); sidx++)
                {
                    const auto &seq = mb.sequence.at (sidx);
                    regChanges[static_cast<uint16_t> (CFGCNT_S00_EXPOTIME + idx)] = seq.expo;
                    regChanges[static_cast<uint16_t> (CFGCNT_S00_PS + idx)] = seq.ps;
                    regChanges[static_cast<uint16_t> (CFGCNT_S00_PLLSET + idx)] = seq.pllset;
                    regChanges[static_cast<uint16_t> (CFGCNT_S00_FRAMERATE + idx)] = seq.fr;

                    //move up to the next LUT assignment register address
                    idx = static_cast<uint16_t> (idx + SEQIDXOFFSET);
                }
            }

            regChanges[CFGCNT_CTRLSEQ] = static_cast<uint16_t> (mbList.front().sequence.size() - 1);
        }
    };
}

ImagerM2450_A11::ImagerM2450_A11 (const ImagerParameters &params) :
    ImagerM2450 (params, 1, 32)
    //base class ctor will raise exception on invalid pointers
    //the PLL calc. for A12 can be used here
    // (not optimal but it was decided that A11 should have only minimal support)
{
    if (params.imageDataTransferType != ImgImageDataTransferType::PIF)
    {
        throw InvalidValue ("The specified interface type is not supported");
    }
    if (params.useSuperframe)
    {
        throw InvalidValue ("Only individual frame mode is supported for the PIF interface");
    }

    if (params.systemFrequency != 26000000)
    {
        throw OutOfBounds ("the imager does not support camera modules with a different system frequency");
    }

    if (params.externalTrigger != ImgTrigger::I2C)
    {
        throw InvalidValue ("The specified trigger mode is not supported");
    }

    m_regTrigger = CFGCNT_TRIG;
    m_valStartTrigger = 1u;
    m_valEndTrigger = 0u;
    m_regStatus = CFGCNT_STATUS;
    m_statusIdle = 1 << 0; // MTCU idle

    m_regPllLutLower = CFGCNT_PLLCFG1_LUT1;
    m_regReconfCnt = MTCU_SEQNUM;
}

ImagerM2450_A11::~ImagerM2450_A11()
{
}


void ImagerM2450_A11::initialize ()
{
    ImagerM2450::initialize ();

    uint16_t regStatus = 0, regDs = 0;

    m_bridge->readImagerRegister (ANAIP_DESIGNSTEP, regDs);

    if (0x0A11 != regDs)
    {
        throw Exception ("wrong design step");
    }

    initialRegisterConfig (BaseConfig);

    const std::map< uint16_t, uint16_t > ifdel{ { CFGCNT_IFDEL, static_cast<uint16_t> (m_imagerParams.interfaceDelay * FSYSCLK) } };

    trackAndWriteRegisters (ifdel);

    const std::map< uint16_t, uint16_t > padconfig
    {
        { ANAIP_PSPADCFG, 0x1513 },
        { CFGCNT_PSOUT, 0x001D },
        { ANAIP_SPARE, 0xEF00 }
    };
    switch (m_imagerParams.illuminationPad)
    {
        case ImgIlluminationPad::SE_P:
            // enable SE_P pad, disable SE_N pad, enable dutycycle control for SE_P output
            trackAndWriteRegisters (padconfig);
            break;
        default:
            throw NotImplemented();
    }

    //internal clock switch
    m_bridge->writeImagerRegister (iSM_CTRL, 1);
    m_bridge->writeImagerRegister (iSM_EN, 1);

    m_bridge->readImagerRegister (MTCU_STATUS, regStatus);

    if (regStatus & 1 << 2)
    {
        throw Exception ("pll locking error");
    }

    m_imagerState = ImagerState::Ready;
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2450_A11::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new PseudoDataInterpreter);
    return pseudoDataInter;
}

std::vector < uint16_t > ImagerM2450_A11::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

void ImagerM2450_A11::reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &frameNumber)
{
    ImagerM2450::reconfigure (useCase, frameNumber);

    ImagerVerificationStatus status = verifyUseCase (useCase);

    if (ImagerVerificationStatus::SUCCESS == status)
    {
        //sequencer polling time will be a half the time a complete sequence needs
        // if there is no frame rate limitation let it be 100ms
        uint32_t frameRate = m_executingUcd.getTargetRate();
        uint32_t timeoutCounter = (500u / frameRate);
        uint16_t valSeqCnt = 0;

        //create map for staging the changes of the exposure time only
        const auto regChanges = prepareUseCase (useCase);

        //poll for the sequence counter
        do
        {
            readImagerRegister (m_regReconfCnt, valSeqCnt);
        }
        while (valSeqCnt && timeoutCounter--);

        if (!timeoutCounter)
        {
            throw Timeout ("sequencer timeout", "stopping the sequencer not succeeded");
        }

        //stop to be sure that the sequencer is not restarting during reconfigure
        writeImagerRegister (m_regTrigger, m_valEndTrigger);

        readImagerRegister (DPRAM_FRAMECNT, frameNumber);

        trackAndWriteRegisters (resolveConfiguration (regChanges));

        //starting the sequencer
        m_executingUcd = useCase;

        writeImagerRegister (m_regTrigger, m_valStartTrigger);
    }
    else
    {
        throw RuntimeError ("reconfiguration failed, not succeeded to verify configuration");
    }
}

void ImagerM2450_A11::startCapture()
{
    ImagerM2450::startCapture();

    uint16_t regValue = 0;

    readImagerRegister (m_regStatus, regValue);

    if (! (regValue & 1 << 0))
    {
        logMessage (ImageSensorLogType::Comment, MSG_CAPTUREBUSY);
        throw LogicError (MSG_CAPTUREBUSY);
    }

    //starting the sequencer
    m_executingUcd = m_preparedUcd;

    writeImagerRegister (m_regTrigger, m_valStartTrigger);

    readImagerRegister (m_regStatus, regValue);

    if (regValue & 1 << 5)
    {
        logMessage (ImageSensorLogType::Comment, MSG_WRONGIF);
        throw RuntimeError (MSG_WRONGIF);
    }

    if (regValue & 1 << 4)
    {
        logMessage (ImageSensorLogType::Comment, MSG_WRONGROI);
        throw RuntimeError (MSG_WRONGROI);
    }

    if (regValue & 1 << 0)
    {
        logMessage (ImageSensorLogType::Comment, MSG_STARTMTCUIDLE);
        throw RuntimeError (MSG_STARTMTCUIDLE);
    }

    m_imagerState = ImagerState::Capturing;
}

void ImagerM2450_A11::stopCapture()
{
    ImagerM2450::stopCapture();

    uint16_t regValue = 0;

    readImagerRegister (m_regStatus, regValue);

    if (regValue & 1 << 0)
    {
        logMessage (ImageSensorLogType::Comment, MSG_STOPMTCUIDLE);
        throw LogicError (MSG_STOPMTCUIDLE);
    }

    shutDownSequencer();

    m_imagerState = ImagerState::Ready;
}

ImagerVerificationStatus ImagerM2450_A11::verifyUseCase (const ImagerUseCaseDefinition &useCase)
{
    if (useCase.getMixedModeEnabled())
    {
        return ImagerVerificationStatus::STREAM_COUNT;
    }

    return Imager::verifyUseCase (useCase);
}

std::map < uint16_t, uint16_t > ImagerM2450_A11::prepareUseCase (const ImagerUseCaseDefinition &useCase)
{
    //assign configuration to measurement blocks
    prepareFrameRateSettings (useCase);
    preparePsSettings (useCase);
    prepareExposureSettings (useCase);

    //create map for staging all changes
    std::map < uint16_t, uint16_t > regChanges;
    prepareModulationSettings (useCase, regChanges);
    SequenceGeneratorM2450_A11().generateMeasurementBlockConfig (m_mbList, regChanges);

    uint16_t roiCMin, roiCMax, roiRMin, roiRMax;
    prepareROI (useCase, roiCMin, roiCMax, roiRMin, roiRMax);
    regChanges[CFGCNT_ROICMINREG] = roiCMin;
    regChanges[CFGCNT_ROICMAXREG] = roiCMax;
    regChanges[CFGCNT_ROIRMINREG] = roiRMin;
    regChanges[CFGCNT_ROIRMAXREG] = roiRMax;

    //config Binning (4 is enable and bit 0=0 defines binningsize=1)
    regChanges[CFGCNT_BINCFG] = 4;

    return regChanges;
}

void ImagerM2450_A11::getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const
{
    //legacy support only (fixed values for a default register configuration)
    ifdel = 1040.0;
    lblank = 132.0;
    cycAdcSocd = 2.;
    cycAdcOddd = 20.;
}
