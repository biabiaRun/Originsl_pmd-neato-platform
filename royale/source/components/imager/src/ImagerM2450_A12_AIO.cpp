/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2450_A12_AIO.hpp>
#include <imager/IImagerModeStrategy.hpp>
#include <imager/M2450_A12/ImagerRegisters.hpp>
#include <imager/M2450_A12/ImagerAIOFirmware.hpp>
#include <imager/M2450_A12/PseudoDataInterpreter_AIO.hpp>
#include <imager/DphyPllStrategyM2450_A12.hpp>
#include <imager/M2450_A12/ImagerBaseConfig.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/MakeUnique.hpp>
#include <common/StringFunctions.hpp>

#include <algorithm>
#include <thread>
#include <cmath>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::imager::M2450_A12;

namespace
{
    const uint16_t SEQIDXOFFSET = 4u;

    //these registers are used to enable specific firmware features
    const uint16_t AIO_SF_ENABLE = 0xC032;
    const uint16_t AIO_MX_ENABLE = 0xC034;
    const uint16_t AIO_SSC_ENABLE = 0xC035;
    const uint16_t AIO_SSC_INIT = 0xC039;
    const uint16_t AIO_WARUMUP_ENABLE = 0xC219;

    //these registers are used if the SSC feature is used
    const uint16_t AIO_SSC_PLLCFG4_LUTx = 0xC24C;
    const uint16_t AIO_SSC_PLLCFG5_LUTx = 0xC24D;
    const uint16_t AIO_SSC_PLLCFG6_LUTx = 0xC24E;
    const uint16_t AIO_SSC_PLLCFG7_LUTx = 0xC24F;
    const uint16_t AIO_SSC_PLLCFG8_LUTx = 0xC250;
    const uint16_t AIO_SSC_PLLCFG_LUT_OFFSET = 5;


    //these registers are the start address for a normal imager configuration,
    //the address is valid for all features, but due to the different meaning
    //of the content a specific label for normal mode (NR) and mixed mode (MX) is used
    const uint16_t AIO_MX_START = 0xC320; //!< start of shadow config area for mixed mode
    const uint16_t AIO_NR_START = 0xC320; //!< start of shadow config area for normal mode

    const uint16_t AIO_MX_MB_OFFSET = 23u;
    const size_t AIO_MX_MAX_PARAMS = 50u;

    const uint16_t AIO_NR_LPFSMFR_1 = 0xC3AC;
    const uint16_t AIO_NR_LPFSMFR_2 = 0xC3AD;
    const uint16_t AIO_NR_CFGCNT_PLLCFG1_LUT1 = 0xC3A0;

    //these registers are used by the safe-reconfig feature
    const auto AIO_SR_TRIGGER = CFGCNT_S31_EXPOTIME;
    const auto AIO_SR_RECONFIG_COUNTER = CFGCNT_S30_PLLSET;
    const auto AIO_SR_RECONFIGFLAGS = CFGCNT_S30_PS;

    //used for the safe-reconfig feature, but specific to normal mode (NR)
    const auto AIO_SR_NR_START = CFGCNT_S20_EXPOTIME;
    const auto AIO_SR_NR_LPFSMFR_1 = CFGCNT_S30_EXPOTIME;
    const auto AIO_SR_NR_LPFSMFR_2 = CFGCNT_S30_FRAMERATE;

    //used for the safe-reconfig feature, but specific to mixed mode (MX)
    const auto AIO_SR_MX_PARAMSTART = CFGCNT_S05_EXPOTIME;
    const auto AIO_SR_MX_DECODESTART = CFGCNT_S05_FRAMERATE;

    class M2450_A12_AIO_Strategy : public IImagerModeStrategy
    {
        const uint16_t AIO_EXPO = 0u;
        const uint16_t AIO_FR = 1u;
        const uint16_t AIO_PS = 2u;
        const uint16_t AIO_PLL = 3u;

    public:
        void generateMeasurementBlockConfig (const std::vector<MeasurementBlock> &mbList,
                                             std::map < uint16_t, uint16_t > &regChanges) override
        {
            uint16_t idx = 0u;

            for (const auto &mb : mbList)
            {
                for (size_t sidx = 0; sidx < mb.sequence.size(); sidx++)
                {
                    const auto &seq = mb.sequence.at (sidx);
                    regChanges[static_cast<uint16_t> (AIO_NR_START + AIO_EXPO + idx)] = seq.expo;
                    regChanges[static_cast<uint16_t> (AIO_NR_START + AIO_PS + idx)] = seq.ps;
                    regChanges[static_cast<uint16_t> (AIO_NR_START + AIO_PLL + idx)] = seq.pllset;
                    regChanges[static_cast<uint16_t> (AIO_NR_START + AIO_FR + idx)] = seq.fr;

                    //move up to the next LUT assignment register address
                    idx = static_cast<uint16_t> (idx + SEQIDXOFFSET);
                }
            }

            regChanges[AIO_NR_LPFSMFR_1] = static_cast<uint16_t> (mbList.front().frameRateCounter >> 16);
            regChanges[AIO_NR_LPFSMFR_2] = static_cast<uint16_t> (mbList.front().frameRateCounter & 0xFFFF);

            //please note that for firmware revisions <= 2065 for the following register the MSB was set, but this is not required anymore
            regChanges[CFGCNT_CTRLSEQ] = static_cast<uint16_t> ( (mbList.front().sequence.size() - 1));
        }

        std::map < uint16_t, uint16_t > reconfigTranslation (const size_t mbCount, const std::map < uint16_t, uint16_t > &regChanges) const override
        {
            std::map <uint16_t, uint16_t> sourceRegisters = regChanges;
            std::map <uint16_t, uint16_t> transformedRegisters;

            const auto translateAndRemove = [&] (const uint16_t address)
            {
                sourceRegisters.erase (address);
                return regChanges.at (address);
            };

            for (size_t sidx = AIO_NR_START; sidx < AIO_NR_CFGCNT_PLLCFG1_LUT1; sidx += SEQIDXOFFSET)
            {
                if (regChanges.count (static_cast<uint16_t> (AIO_EXPO + sidx)))
                {
                    transformedRegisters[static_cast<uint16_t> (AIO_SR_NR_START + AIO_EXPO + ( (sidx - AIO_NR_START) / 2))] =
                        translateAndRemove (static_cast<uint16_t> (AIO_EXPO + sidx));
                }

                if (regChanges.count (static_cast<uint16_t> (AIO_FR + sidx)))
                {
                    transformedRegisters[static_cast<uint16_t> (AIO_SR_NR_START + AIO_FR + ( (sidx - AIO_NR_START) / 2))] =
                        translateAndRemove (static_cast<uint16_t> (AIO_FR + sidx));
                }
            }

            if (regChanges.count (AIO_NR_LPFSMFR_1))
            {
                transformedRegisters[AIO_SR_NR_LPFSMFR_1] = translateAndRemove (AIO_NR_LPFSMFR_1);
            }

            if (regChanges.count (AIO_NR_LPFSMFR_2))
            {
                transformedRegisters[AIO_SR_NR_LPFSMFR_2] = translateAndRemove (AIO_NR_LPFSMFR_2);
            }

            if (regChanges.count (CFGCNT_CTRLSEQ))
            {
                //this register is forced to be written for normal stop/start cycles but there
                //is no need to write it for reconfigurations as it is guaranted that the value
                //will not change (only exposure and framerates may change)
                sourceRegisters.erase (CFGCNT_CTRLSEQ);
            }

            if (sourceRegisters.size())
            {
                throw RuntimeError ("reconfiguration missed some registers to translate");
            }

            //it would be more clear to have this as separate write command, but it is also possible
            //to add this to the list of translated registers; the reason why it is added is that
            //in the mixed mode case it is required to add it, so it is added here too for convenience
            transformedRegisters[AIO_SR_RECONFIGFLAGS] = 1u;

            return transformedRegisters;
        }
    };

    class M2450_A12_MX_Strategy : public IImagerModeStrategy
    {
        const uint16_t MM_EXPO = 0u;
        const uint16_t MM_FR = 1u;
        const uint16_t MM_PS = 2u;
        const uint16_t MM_PLL = 3u;
        const uint16_t MM_MB_LPFSMFR_1 = 20u;
        const uint16_t MM_MB_LPFSMFR_2 = 21u;
        const uint16_t MM_MB_CTRLSEQ = 22u;
        const uint16_t MM_REPEAT_START = 0xC318;

    public:
        void generateMeasurementBlockConfig (const std::vector<MeasurementBlock> &mbList,
                                             std::map < uint16_t, uint16_t > &regChanges) override
        {
            size_t mbCnt = 0;
            uint16_t curMBStart = AIO_MX_START;

            for (const auto &mb : mbList)
            {
                uint16_t idx = 0u;

                for (size_t sidx = 0; sidx < mb.sequence.size(); sidx++)
                {
                    const auto &seq = mb.sequence.at (sidx);
                    regChanges[static_cast<uint16_t> (curMBStart + MM_EXPO + idx)] = seq.expo;
                    regChanges[static_cast<uint16_t> (curMBStart + MM_PS + idx)] = seq.ps;
                    regChanges[static_cast<uint16_t> (curMBStart + MM_PLL + idx)] = seq.pllset;
                    regChanges[static_cast<uint16_t> (curMBStart + MM_FR + idx)] = seq.fr;

                    //move up to the next sequence entry register address of the current measurement block
                    idx = static_cast<uint16_t> (idx + SEQIDXOFFSET);
                }

                if (mb.sequence.size())
                {
                    mbCnt++;
                    regChanges[static_cast<uint16_t> (curMBStart + MM_MB_LPFSMFR_1)] =
                        static_cast<uint16_t> (mb.frameRateCounter >> 16);
                    regChanges[static_cast<uint16_t> (curMBStart + MM_MB_LPFSMFR_2)] =
                        static_cast<uint16_t> (mb.frameRateCounter & 0xFFFF);

                    regChanges[static_cast<uint16_t> (curMBStart + MM_MB_CTRLSEQ)] =
                        static_cast<uint16_t> ( (mb.sequence.size() - 1) | (mb.safeForReconfig ? 0x8000 : 0));
                }

                //move up to the next measurement block register address
                curMBStart = static_cast<uint16_t> (curMBStart + AIO_MX_MB_OFFSET);
            }

            for (size_t adrOffset = 0; adrOffset < mbList.size(); adrOffset++)
            {
                regChanges[static_cast<uint16_t> (MM_REPEAT_START + adrOffset)] = mbCnt > adrOffset ? mbList[adrOffset].cycles : 0u;
            }
        }

        std::map < uint16_t, uint16_t > reconfigTranslation (const size_t mbCount, const std::map < uint16_t, uint16_t > &regChanges) const override
        {
            return reconfigTranslationMixedMode (
                       mbCount,
                       AIO_MX_START,
                       AIO_MX_MB_OFFSET,
                       SEQIDXOFFSET,
                       AIO_SR_RECONFIGFLAGS,
                       AIO_MX_MAX_PARAMS,
                       AIO_SR_MX_PARAMSTART,
                       AIO_SR_MX_DECODESTART,
                       regChanges);
        }
    };
}

ImagerM2450_A12_AIO::ImagerM2450_A12_AIO (const ImagerParameters &params) :
    ImagerM2450 (params, 1, 20),
    m_currentModeIsMixedMode (false),
    m_strategy (makeUnique<M2450_A12_AIO_Strategy>())
{
    if (params.imageDataTransferType != ImgImageDataTransferType::PIF &&
            params.imageDataTransferType != ImgImageDataTransferType::MIPI_1LANE &&
            params.imageDataTransferType != ImgImageDataTransferType::MIPI_2LANE)
    {
        throw InvalidValue ("The specified interface type is not supported");
    }

    if (params.systemFrequency < 10000000 ||
            params.systemFrequency > 35000000 ||
            params.systemFrequency % 100000 != 0)
    {
        throw OutOfBounds ("the imager does not support camera modules with this system frequency");
    }

    if (! (params.externalTrigger == ImgTrigger::I2C ||
            params.externalTrigger == ImgTrigger::GPIO13 ||
            params.externalTrigger == ImgTrigger::GPIO14))
    {
        throw InvalidValue ("The specified trigger mode is not supported");
    }

    if (params.imageDataTransferType == ImgImageDataTransferType::PIF &&
            params.useSuperframe)
    {
        throw InvalidValue ("Superframes can only be used with CSI2");
    }

    m_useSuperFrame = params.useSuperframe;
    m_regTrigger = AIO_SR_TRIGGER;
    m_valStartTrigger = 1u;
    m_valEndTrigger = 0u;
    m_regReconfCnt = AIO_SR_RECONFIG_COUNTER;
    m_regStatus = CFGCNT_STATUS;
    m_statusIdle = 1 << 15; //AOI firmware LPFSM idle bit
}

ImagerM2450_A12_AIO::~ImagerM2450_A12_AIO()
{
}

std::vector < uint16_t > ImagerM2450_A12_AIO::getSerialRegisters()
{
    std::vector < uint16_t > efuseValues (4);
    m_bridge->readImagerBurst (ANAIP_EFUSEVAL1, efuseValues);
    return efuseValues;
}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2450_A12_AIO::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new PseudoDataInterpreter_AIO);
    return pseudoDataInter;
}

std::vector<MeasurementBlock> ImagerM2450_A12_AIO::createMeasurementBlockList (const ImagerUseCaseDefinition &useCase) const
{
    if (useCase.getMixedModeEnabled())
    {
        return std::vector<MeasurementBlock> (8, MeasurementBlock (5));
    }

    return std::vector<MeasurementBlock> (m_defaultMeasurementBlockCount, MeasurementBlock (m_defaultMeasurementBlockCapacity));
}

void ImagerM2450_A12_AIO::initialize()
{
    ImagerM2450::initialize();

    //do not use predefined LUT assignment
    m_lutAssignment.clear();

    uint16_t regValue = 0, regDs = 0;

    readImagerRegister (ANAIP_DESIGNSTEP, regDs);

    if (0x0A12 != regDs)
    {
        throw Exception ("wrong design step");
    }

    uint16_t regEfuse = 0;

    readImagerRegister (ANAIP_EFUSEVAL1, regEfuse);

    if (regEfuse & 1 << 2)
    {
        //values according to devspec M2450 2.0 (Updated rows of 19k ROI from 120 to 121 (to allow for additional pseudo data)
        m_columnLimitSensor = 176;
        m_rowLimitSensor = 121;
    }

    //dphy pll configuration if system frequency deviates from default
    if (m_imagerParams.systemFrequency != 26000000)
    {
        DphyPllStrategyM2450_A12 pllCalc (m_imagerParams.systemFrequency);
        std::vector<uint16_t> pllcfg (8);

        if (!pllCalc.pllSettings (FSYSCLK * 3.0, false, pllcfg))
        {
            throw Exception ("Invalid dphy pll frequency specified");
        }

        logMessage (ImageSensorLogType::Comment, "Reconfiguration of DPHY PLL to " +
                    toStdString (m_imagerParams.systemFrequency) + "Hz");

        writeImagerBurst (ANAIP_DPHYPLLCFG1, pllcfg);
    }

    initialRegisterConfig (BaseConfig);

    //the interface delay will be set depending on the time specified in the module configuration
    //and the frame blanking value must be set to 8 at all times
    const std::map< uint16_t, uint16_t > ifdel{ {
            CFGCNT_IFDEL, 0x4000 |
            static_cast<uint16_t> (m_imagerParams.interfaceDelay * FSYSCLK)
        } };
    trackAndWriteRegisters (ifdel);

    uint16_t regAnaipSpare = 0xEF00;

    if (m_regDownloaded.count (ANAIP_SPARE))
    {
        if (static_cast<uint16_t> (m_regDownloaded[ANAIP_SPARE] & 0xEF00) == regAnaipSpare)
        {
            regAnaipSpare = m_regDownloaded[ANAIP_SPARE];
        }
        else
        {
            throw Exception ("invalid base config for ANAIP_SPARE");
        }
    }

    if (m_regDownloaded.count (CFGCNT_PSOUT))
    {
        throw Exception ("CFGCNT_PSOUT must not be set by a base config");
    }

    const std::map< uint16_t, uint16_t > padconfig_SE_P
    {
        { ANAIP_PSPADCFG, 0x1513 },
        { CFGCNT_PSOUT, 0x031D }, //ROYAL-1913 disable LVDS and enable internal MOD_SE_P and MOD_SE_N signal
        { ANAIP_SPARE, regAnaipSpare }
    };

    const std::map< uint16_t, uint16_t > padconfig_LVDS
    {
        { CFGCNT_PSOUT, 0x0133 }, // enable LVDS signal
        { ANAIP_PSLVDSCFG, 0x0001 }, // LVDS always on
        { ANAIP_SPARE, regAnaipSpare }
    };

    switch (m_imagerParams.illuminationPad)
    {
        case ImgIlluminationPad::SE_P:
            // enable SE_P pad, disable SE_N pad, enable dutycycle control for SE_P output
            trackAndWriteRegisters (padconfig_SE_P);
            break;
        case ImgIlluminationPad::LVDS:
            // enable LVDS, disable SE_P pad, disable SE_N pad
            trackAndWriteRegisters (padconfig_LVDS);
            break;
        default:
            throw NotImplemented();
    }

    switch (m_imagerParams.imageDataTransferType)
    {
        case ImgImageDataTransferType::PIF:
            trackAndWriteRegisters ({ { CFGCNT_PIFCCFG, 0x1057 }, { CFGCNT_CSICFG, 0x0280 } });
            break;
        case ImgImageDataTransferType::MIPI_1LANE:
            if (m_useSuperFrame)
            {
                //use short packets and disable pseudo data because it is generated by the firmware
                trackAndWriteRegisters ({ { CFGCNT_PIFCCFG, 0x000 }, { CFGCNT_CSICFG, 0x0201 } });
            }
            else
            {
                //use short packets and transmit pseudo data as first packet before the first line
                trackAndWriteRegisters ({ { CFGCNT_PIFCCFG, 0x000 }, { CFGCNT_CSICFG, 0x0211 } });
            }
            break;
        case ImgImageDataTransferType::MIPI_2LANE:
            if (m_useSuperFrame)
            {
                //use short packets and disable pseudo data because it is generated by the firmware
                trackAndWriteRegisters ({ { CFGCNT_PIFCCFG, 0x000 }, { CFGCNT_CSICFG, 0x0081 } });
            }
            else
            {
                //use short packets and transmit pseudo data as first packet before the first line
                trackAndWriteRegisters ({ { CFGCNT_PIFCCFG, 0x000 }, { CFGCNT_CSICFG, 0x0091 } });
            }
            break;
        default:
            throw NotImplemented();
            break;
    }

    switch (m_currentTrigger)
    {
        case ImgTrigger::GPIO13:
            maskedWriteRegister (ANAIP_GPIOMUX4, 0x03E0, 0x0180, 0x0000);
            maskedWriteRegister (ANAIP_PADGPIOCFG6, 0xFF00, 0x1A00, 0x1313);
            break;
        case ImgTrigger::GPIO14:
            maskedWriteRegister (ANAIP_GPIOMUX4, 0x7C00, 0x3000, 0x0000);
            maskedWriteRegister (ANAIP_PADGPIOCFG7, 0x00FF, 0x001A, 0x1313);
            break;
        case ImgTrigger::I2C:
        default:
            break;
    }

    //internal clock switch and wait until ism ready
    writeImagerRegister (iSM_CTRL, 1);
    writeImagerRegister (iSM_EN, 1);
    m_bridge->sleepFor (std::chrono::microseconds (200));
    readImagerRegister (iSM_ISMSTATE, regValue);

    if (regValue & 1 << 14)
    {
        throw RuntimeError ("iSMx is currently enabled");
    }

    readImagerRegister (MTCU_STATUS, regValue);

    if (regValue & 1 << 2)
    {
        throw Exception ("pll locking error");
    }

    startFirmware();

    m_imagerState = ImagerState::Ready;
}

void ImagerM2450_A12_AIO::startFirmware()
{
    //upload and activate imager firmware
    writeImagerRegister (iSM_CTRL, 2);

    loadFirmware (AIOFirmware_Version, AIOFirmware_Page1, AIOFirmware_Page2);

    //copy ANAIP default values into FW RAM SSC area
    writeImagerRegister (AIO_SSC_INIT, 1u); // this is a write clear bit
    writeImagerRegister (AIO_MX_ENABLE, 2u);
    writeImagerRegister (AIO_SF_ENABLE, m_useSuperFrame ? 1u : 0u);
    writeImagerRegister (AIO_SSC_ENABLE, 0u);
    writeImagerRegister (AIO_WARUMUP_ENABLE, 0u);
    writeImagerRegister (m_regTrigger, m_valEndTrigger); //don't start capturing
    writeImagerRegister (iSM_EN, 1u); // <-- start AllInOne firmware
    m_bridge->sleepFor (std::chrono::microseconds (80));
    writeImagerRegister (iSM_EN, 2u); // <-- stop AllInOne firmware

    uint16_t regValue = 0;
    readImagerRegister (iSM_ISMSTATE, regValue);

    if (regValue & (1 << 14))
    {
        throw RuntimeError ("iSM still active");
    }
}

void ImagerM2450_A12_AIO::loadFirmware (const uint32_t version,
                                        const std::map<uint16_t, uint16_t> &page1,
                                        const std::map<uint16_t, uint16_t> &page2)
{
    const uint32_t bytecode_version_msb = page1.at (iSM_FW_RAM_VERSION_MSB) & 0x7ff;
    const uint32_t bytecode_version_lsb = page1.at (iSM_FW_RAM_VERSION_LSB) & 0x7ff;

    if (bytecode_version_msb & ~0x1ff)
    {
        //upper bits must be zero (otherwise a not supported versioning system is used)
        throw RuntimeError ("firmware version system not supported");
    }

    const uint32_t bytecode_version = (bytecode_version_msb << 11) + bytecode_version_lsb;

    if (bytecode_version != version)
    {
        throw RuntimeError ("firmware version not supported");
    }

    if (!page1.empty())
    {
        writeImagerRegister (iSM_MEMPAGE, 1 << 6);

        //write page 1
        trackAndWriteRegisters (page1);
    }

    if (!page2.empty())
    {
        writeImagerRegister (iSM_MEMPAGE, static_cast<uint16_t> (1 << 6 | 1 << 5));

        //write page 2
        trackAndWriteRegisters (page2);
    }

    writeImagerRegister (iSM_MEMPAGE, 0x0000);
}

void ImagerM2450_A12_AIO::startCapture()
{
    ImagerM2450::startCapture();

    uint16_t regValue = 0;

    readImagerRegister (m_regStatus, regValue);

    if (! (regValue & m_statusIdle))
    {
        logMessage (ImageSensorLogType::Comment, MSG_CAPTUREBUSY);
        throw LogicError (MSG_CAPTUREBUSY);
    }

    //starting the sequencer
    m_executingUcd = m_preparedUcd;

    const auto enabledMixedMode = m_executingUcd.getMixedModeEnabled();
    const auto enabledSSC = m_executingUcd.getSSCEnabled();
    const bool extTriggerUsed = (m_currentTrigger != ImgTrigger::I2C);

    switch (m_currentTrigger)
    {
        //I2C start trigger is replaced by trigger via GPIO13
        case ImgTrigger::GPIO13:
            maskedWriteRegister (ANAIP_GPIOMUX4, 0x03E0, 0x00A0, 0x0000); //!<  enable the input mux (ism external irq)
            break;
        //I2C start trigger is replaced by trigger via GPIO14
        case ImgTrigger::GPIO14:
            maskedWriteRegister (ANAIP_GPIOMUX4, 0x7C00, 0x1400, 0x0000); //!<  enable the input mux (ism external irq)
            break;
        default:
            break;
    }

    writeImagerRegister (AIO_MX_ENABLE, enabledMixedMode ? 1u : 0u); //enable mixed mode
    writeImagerRegister (AIO_SSC_ENABLE, enabledSSC ? 1u : 0u); //copy of ssc registers to ANAIP on/off

    if (!extTriggerUsed)
    {
        writeImagerRegister (m_regTrigger, m_valStartTrigger); //enable trigger (replaced MTCU trigger)
    }

    writeImagerRegister (iSM_EN, 1u); // <-- start AllInOne firmware

    //firmware copies the shadow-config to safe-reconfig, this takes some time...
    m_bridge->sleepFor (std::chrono::microseconds (320));

    //DMEM read access is not safe (skip checking iSM state)
    //the following lines just document the normal way to check for successful firmware startup
    // but as this register is not safe to be read the sleep in the previous line is done
    //readImagerRegister (iSM_ISMSTATE, regValue);
    //if (! (regValue & (1 << 14)))
    //{
    //   throw RuntimeError ("iSM inactive");
    //}

    checkPostStartStatus (extTriggerUsed);

    m_imagerState = ImagerState::Capturing;
}

void ImagerM2450_A12_AIO::checkPostStartStatus (bool extTriggerUsed)
{
    uint16_t regValue = 0;

    //checking the config will take ca ~1us and if config okay the pll lock will (or will not) happen after ~200us
    m_bridge->sleepFor (std::chrono::microseconds (200));

    readImagerRegister (MTCU_STATUS, regValue);

    if (regValue & 1 << 2)
    {
        logMessage (ImageSensorLogType::Comment, "error modpll lock");
        throw RuntimeError ("error modpll lock");
    }

    if (!extTriggerUsed)
    {
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

        if (regValue & m_statusIdle)
        {
            logMessage (ImageSensorLogType::Comment, MSG_STARTMTCUIDLE);
            throw RuntimeError (MSG_STARTMTCUIDLE);
        }
    }
}

void ImagerM2450_A12_AIO::reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex)
{
    Imager::reconfigure (useCase, reconfigIndex);

    ImagerVerificationStatus status = verifyUseCase (useCase);

    if (ImagerVerificationStatus::SUCCESS == status)
    {
        //this method is most probably accessed directly by processing from a different thread,
        //as the processing implementation is unknown this is a critical section
        std::unique_lock <std::mutex> lock (m_reconfigLock);

        //before a new reconfig-trigger can be done the previous reconfig must have completed
        uint16_t flagStatus;
        m_bridge->readImagerRegister (AIO_SR_RECONFIGFLAGS, flagStatus);

        if (flagStatus & 3u)
        {
            commitOrRollbackShadowedRegisters (false);
            throw RuntimeError ("reconfiguration failed, firmware register transfer not successful");
        }
        commitOrRollbackShadowedRegisters (true);

        //create map for staging the changes of exposure time and/or frame rate
        const auto regChanges = resolveConfiguration (prepareUseCase (useCase));

        //get index which marks the previous UCD
        m_bridge->readImagerRegister (m_regReconfCnt, reconfigIndex);

        //write the changes into the reconfiguration register space
        //note that the config-done trigger is part of the translated
        trackAndWriteRegisters (m_strategy->reconfigTranslation (m_mbList.size(), regChanges));

        //only translated registers are tracked, if the full reconfiguration is done track also the shadowed registers...
        trackShadowedRegisters (regChanges); //these register will be written by the firmware, so just track them

        m_executingUcd = useCase;
    }
    else
    {
        throw RuntimeError ("reconfiguration failed, not succeeded to verify configuration");
    }
}

void ImagerM2450_A12_AIO::stopCapture()
{
    //for the AllInOne firmware revision > 2065 ensure that no config copy operation is pending;
    //the reconfigure method is most probably accessed directly by processing from a different thread,
    //so it could trigger a config copy operation while this method is checking the flag - thus the lock
    std::unique_lock <std::mutex> lock (m_reconfigLock);

    uint16_t flagStatus;
    m_bridge->readImagerRegister (AIO_SR_RECONFIGFLAGS, flagStatus);

    if (flagStatus & 3u)
    {
        //in case a reconfiguration is pending wait some time to let it happen before stopping
        m_bridge->sleepFor (std::chrono::microseconds (2 * 1000 * getMaxSafeReconfigTimeMilliseconds (m_executingUcd)));
        // we don't know whether the registers were written or not
        commitOrRollbackShadowedRegisters (false);
    }
    else
    {
        commitOrRollbackShadowedRegisters (true);
    }

    //call the common stop procedure (set register address m_regTrigger to value m_valEndTrigger)
    ImagerM2450::stopCapture();

    uint16_t regValue = 0;

    switch (m_currentTrigger)
    {
        //the external trigger signal might have been released already earlier,
        //but disabling the trigger input will definitely stop capturing
        case ImgTrigger::GPIO13:
            maskedWriteRegister (ANAIP_GPIOMUX4, 0x03E0, 0x0180, 0x0000);
            break;
        case ImgTrigger::GPIO14:
            maskedWriteRegister (ANAIP_GPIOMUX4, 0x7C00, 0x3000, 0x0000);
            break;
        //in case of I2C triggering we expect the MTCU to be busy
        case ImgTrigger::I2C:
        default:
            readImagerRegister (m_regStatus, regValue);

            if (regValue & m_statusIdle)
            {
                //if MTCU is idle capturing ended unexpectedly (e.g. due to reset by eye-safety circuit)
                logMessage (ImageSensorLogType::Comment, MSG_STOPMTCUIDLE);
                throw LogicError (MSG_STOPMTCUIDLE);
            }
            break;
    }

    shutDownSequencer();

    m_imagerState = ImagerState::Ready;
}

void ImagerM2450_A12_AIO::shutDownSequencer()
{
    Imager::shutDownSequencer();

    //iSM must not have any copy operation pending
    uint16_t flagStatus;
    m_bridge->readImagerRegister (AIO_SR_RECONFIGFLAGS, flagStatus);

    if (flagStatus & 3u)
    {
        commitOrRollbackShadowedRegisters (false);
        throw RuntimeError ("stop failed, firmware register transfer still pending");
    }
    commitOrRollbackShadowedRegisters (true);

    writeImagerRegister (iSM_EN, 2u); // <-- stop AllInOne firmware

    uint16_t regValue;
    m_bridge->sleepFor (std::chrono::microseconds (10));
    readImagerRegister (iSM_ISMSTATE, regValue);

    if (regValue & (1 << 14))
    {
        throw RuntimeError ("iSM active");
    }
}

void ImagerM2450_A12_AIO::getReadoutDelays (double &ifdel, double &lblank, double &cycAdcSocd, double &cycAdcOddd) const
{
    if (m_regDownloaded.count (CFGCNT_IFDEL))
    {
        ifdel = (double) (static_cast<uint16_t> (m_regDownloaded.at (CFGCNT_IFDEL) & 0x7FF));
    }
    else
    {
        ifdel = 0.0;
    }

    if (m_regDownloaded.count (CFGCNT_ROS1))
    {
        cycAdcSocd = 2.0 * (1.0 + (double) (static_cast<uint16_t> ( (m_regDownloaded.at (CFGCNT_ROS1) >> 11) & 3)));
        cycAdcOddd = 20.0 + 2.0 * ( (double) (static_cast<uint16_t> ( (m_regDownloaded.at (CFGCNT_ROS1) >> 13) & 7)));
        const double lblankTime = (double) (static_cast<uint16_t> (m_regDownloaded.at (CFGCNT_ROS1) & 0x3F));
        const double lowro = std::pow (2.0, ( (double) (static_cast<uint16_t> ( (m_regDownloaded.at (CFGCNT_ROS1) >> 6) & 3))));

        lblank = lowro * (cycAdcSocd + cycAdcOddd) * lblankTime;
    }
    else
    {
        lblank = 132.0;
        cycAdcSocd = 2.;
        cycAdcOddd = 20.;
    }
}

bool ImagerM2450_A12_AIO::isValidExposureTime (bool enabledMixedMode, size_t overallRawFrameCount, uint32_t exposureTime, uint32_t modFreq)
{
    if (enabledMixedMode)
    {
        //assume worst case that is supported (no partial reconfig support > 51 params);
        //that is a change of exposure (x)or framerate for each raw frame plus two parameters
        //for the overall framerate - no ps/pll or CTRLSEQ change is supported;
        const size_t numParamCountMax = std::min (2 + (overallRawFrameCount * 2), (size_t) 51);
        const auto mixedModeReconfigWorstCase = static_cast<double> (numParamCountMax) * 2.4;
        const auto minExpoMixedMode = static_cast<uint32_t> (12u + mixedModeReconfigWorstCase);

        if (exposureTime < minExpoMixedMode)
        {
            return false;
        }
    }
    else
    {
        const uint32_t minExpoNormalMode = 27u;

        if (exposureTime < minExpoNormalMode)
        {
            return false;
        }
    }

    return ImagerM2450::isValidExposureTime (enabledMixedMode, overallRawFrameCount, exposureTime, modFreq);
}


ImagerVerificationStatus ImagerM2450_A12_AIO::verifyRegion (const ImagerUseCaseDefinition &useCase)
{
    uint16_t columns, rows;
    useCase.getImage (columns, rows);

    if (columns < PseudoDataInterpreter::RECONFIG_INDEX)
    {
        return ImagerVerificationStatus::REGION;
    }

    return ImagerM2450::verifyRegion (useCase);
}

ImagerVerificationStatus ImagerM2450_A12_AIO::verifySSCSettings (const ImagerUseCaseDefinition &useCase)
{
    //Frequencies and general SSC parameter limits are validated with verifyModulationSettings.
    //At this place there could be a validation if the SSC parameters
    //are okay, however no formula is yet provided by R&D for such
    //a calculation, it is currently up to the UCD creator to use
    //meaningful values.

    return ImagerVerificationStatus::SUCCESS;
}

void ImagerM2450_A12_AIO::prepareSSCSettings (const uint16_t lutIndex,
        const std::vector<uint16_t> &pllCfg,
        std::map < uint16_t, uint16_t > &regChanges)
{
    //configure the SSC FW for this LUT
    regChanges[static_cast<uint16_t> (AIO_SSC_PLLCFG4_LUTx + (AIO_SSC_PLLCFG_LUT_OFFSET * lutIndex))] = pllCfg.at (3);
    regChanges[static_cast<uint16_t> (AIO_SSC_PLLCFG5_LUTx + (AIO_SSC_PLLCFG_LUT_OFFSET * lutIndex))] = pllCfg.at (4);
    regChanges[static_cast<uint16_t> (AIO_SSC_PLLCFG6_LUTx + (AIO_SSC_PLLCFG_LUT_OFFSET * lutIndex))] = pllCfg.at (5);
    regChanges[static_cast<uint16_t> (AIO_SSC_PLLCFG7_LUTx + (AIO_SSC_PLLCFG_LUT_OFFSET * lutIndex))] = pllCfg.at (6);
    regChanges[static_cast<uint16_t> (AIO_SSC_PLLCFG8_LUTx + (AIO_SSC_PLLCFG_LUT_OFFSET * lutIndex))] = pllCfg.at (7);
}

void ImagerM2450_A12_AIO::prepareMeasurementBlockTargetTime (MeasurementBlockId mbId,
        const double mbTargetTime,
        const double mbMeasurementTime)
{
    //due to ROYAL-2210 the low power feature is not yet enabled
    m_mbList[mbId].frameRateCounter = 0u;
}

std::map < uint16_t, uint16_t > ImagerM2450_A12_AIO::prepareUseCase (const ImagerUseCaseDefinition &useCase)
{
    const auto newMode = useCase.getMixedModeEnabled();
    const auto modeChanged = (newMode != m_currentModeIsMixedMode);
    m_currentModeIsMixedMode = newMode;

    if (newMode)
    {
        if (modeChanged)
        {
            //when switching to mixed-mode the content of the CFGCNT PLL registers must be
            //invalidated as it could have been changed by the firmware
            for (uint16_t reg = CFGCNT_PLLCFG1_LUT1; reg <= CFGCNT_PLLCFG3_LUT4; reg = static_cast < uint16_t > (reg + 1))
            {
                m_regDownloaded.erase (reg);
            }
        }

        /**
        * The mixed mode firmware does not have a dedicated safe reconfig shadow area for the PLL setting.
        * This means that it is not possible to change modulation frequencies while capturing.
        * Be aware that disabling the mixed mode (enable interleaved bit = 0) would require to change this
        * address to 928d as the normal safe-reconfig firmware is copying these registers.
        */
        m_regPllLutLower = CFGCNT_PLLCFG1_LUT1;
    }
    else
    {
        m_regPllLutLower = AIO_NR_CFGCNT_PLLCFG1_LUT1;
    }

    if (modeChanged)
    {
        if (newMode)
        {
            m_strategy = makeUnique<M2450_A12_MX_Strategy>();
        }
        else
        {
            m_strategy = makeUnique<M2450_A12_AIO_Strategy>();
        }

        //location of PLL LUTs changed, invalidate the assignment
        m_lutAssignment.clear();
    }

    //force write (firmware possibly changed the register value)
    //- this is required because the shadow-config area of the firmware's
    //  normal mode hasn't an entry for CTRLSEQ, therefore no initial copy
    //  from the shadow-config area into the CFGCNT are is done and in the
    //  CFGCNT an old value from a previous run of a mixed mode use case
    //  could still be there - it is not possible to guess which value it is
    //  and it would be necessary to read the register (to avoid this read
    //  the tracked address will be cleared to force a write)
    m_regDownloaded.erase (CFGCNT_CTRLSEQ);

    prepareFrameRateSettings (useCase);
    preparePsSettings (useCase);
    prepareExposureSettings (useCase);

    //create map for staging all changes
    std::map < uint16_t, uint16_t > regChanges;
    prepareModulationSettings (useCase, regChanges);

    m_strategy->generateMeasurementBlockConfig (m_mbList, regChanges);

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
