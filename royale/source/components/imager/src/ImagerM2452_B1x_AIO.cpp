/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>

#include <imager/ImagerM2452_B1x_AIO.hpp>
#include <imager/IImagerModeStrategy.hpp>
#include <imager/M2452/ImagerRegisters.hpp>
#include <imager/M2452_B1x/ImagerBaseConfig.hpp>
#include <imager/M2452_B1x/ImagerAIOFirmware.hpp>
#include <imager/M2452/PseudoDataInterpreter_AIO.hpp>

#include <limits>
#include <thread>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::imager::M2452; //!< The register maps for A11 and B11/B12 are equivalent

namespace
{
    const uint16_t PLLCFG4_SSC_LUTx = 0xb398;
    const uint16_t PLLCFG5_SSC_LUTx = 0xb399;
    const uint16_t PLLCFG6_SSC_LUTx = 0xb39a;
    const uint16_t PLLCFG7_SSC_LUTx = 0xb39b;
    const uint16_t PLLCFG8_SSC_LUTx = 0xb39c;
    const uint16_t PLLCFG_SSC_LUT_OFFSET = 5;

    const uint16_t SEQIDXOFFSET = 3u;

    const uint16_t AIO_SSC_INIT = 0xb3ad;
    const uint16_t AIO_CFGCNT_PLLCFG1_LUT1 = 0xb334;

    const auto AIO_NM_START = CFGCNT_S11_EXPOTIME;
    const auto AIO_SR_RECONFIG_COUNTER = CFGCNT_S16_EXPOTIME;
    const uint16_t AIO_MM_MB_START = 0xB300;
    const uint16_t AIO_MM_MB_OFFSET = 19u;
    const size_t AIO_MM_MAX_PARAMS = 15u;
    const auto AIO_MM_SR_DECODESTART = CFGCNT_S05_FRAMERATE;
    const auto AIO_MM_SR_PARAMSTART = CFGCNT_S05_EXPOTIME;
    const auto AIO_SR_RECONFIGFLAGS = CFGCNT_S15_PS;

    void setLowFpsFrameRate (std::map < uint16_t, uint16_t > &regChanges,
                             const uint32_t frameRateCounter,
                             const uint16_t lpfsmFrameRate,
                             const uint16_t regCounterReload,
                             const uint16_t regInserts)
    {
        //new LPFSM counter value will be set to an imaginary framerate about three times the specified frame rate
        regChanges[lpfsmFrameRate] = narrow_cast<uint16_t> (frameRateCounter / 3u);

        //the low framerate counter will be set to a time matching the LPFSM framerate divided by three,
        //but as the low framerate counter runs with half the speed of the LPFSM counter it must be
        //multiplied by two to match the LPFSM counter time.
        regChanges[regCounterReload] = narrow_cast<uint16_t> (regChanges[lpfsmFrameRate] * 2u / 3u);

        //the first counter overrun equals one third of the LPFSM counter overrun,
        //so after the first counter overrun insert six low framerate counter runs to
        //achieve a framerate according to the original frameRateCounter value.
        //After the inserts the LPFSM counter will do the remaining 2/3 of its counting.
        //
        // Timing:
        // |=counting, i=counting(insert) .=not counting
        //  LPFSM: | . . . . . . | |
        //  LOWFR: | i i i i i i . .
        //
        regChanges[regInserts] = 2u * 3u; //this enables the low framerate feature of the firmware
    }

    class M2452_B1x_MM_Strategy : public IImagerModeStrategy
    {
        const uint16_t MM_EXPO = 0u;
        const uint16_t MM_FR = 1u;
        const uint16_t MM_PS = 2u;
        const uint16_t MM_SEQIDXOFFSET = 3u;
        const uint16_t MM_MB_LPFSMFR = 15u;
        const uint16_t MM_MB_LOWFPS_COUNTERRELOAD = 16u;
        const uint16_t MM_MB_LOWFPS_INSERTS = 17u;
        const uint16_t MM_MB_CTRLSEQ = 18u;
        const uint16_t MM_REPEAT_START = 0xB2F8;

    public:
        void generateMeasurementBlockConfig (const std::vector<MeasurementBlock> &mbList,
                                             std::map < uint16_t, uint16_t > &regChanges) override
        {
            size_t mbCnt = 0;
            uint16_t curMBStart = AIO_MM_MB_START;

            for (const auto &mb : mbList)
            {
                uint16_t idx = 0u;

                for (size_t sidx = 0; sidx < mb.sequence.size(); sidx++)
                {
                    const auto &seq = mb.sequence.at (sidx);
                    regChanges[static_cast<uint16_t> (curMBStart + MM_EXPO + idx)] = seq.expo;
                    regChanges[static_cast<uint16_t> (curMBStart + MM_PS + idx)] = static_cast<uint16_t> (seq.ps | seq.pllset << 9);
                    regChanges[static_cast<uint16_t> (curMBStart + MM_FR + idx)] = seq.fr;

                    //move up to the next sequence entry register address of the current measurement block
                    idx = static_cast<uint16_t> (idx + MM_SEQIDXOFFSET);
                }

                if (mb.sequence.size())
                {
                    mbCnt++;

                    if (mb.frameRateCounter > std::numeric_limits<uint16_t>::max())
                    {
                        //if the LPFSM framerate counter cannot handle the specified
                        ///target framerate use the low-framerate firmware feature
                        setLowFpsFrameRate (regChanges,
                                            mb.frameRateCounter,
                                            static_cast<uint16_t> (curMBStart + MM_MB_LPFSMFR),
                                            static_cast<uint16_t> (curMBStart + MM_MB_LOWFPS_COUNTERRELOAD),
                                            static_cast<uint16_t> (curMBStart + MM_MB_LOWFPS_INSERTS));
                    }
                    else
                    {
                        regChanges[static_cast<uint16_t> (curMBStart + MM_MB_LPFSMFR)] = static_cast<uint16_t> (mb.frameRateCounter);
                        regChanges[static_cast<uint16_t> (curMBStart + MM_MB_LOWFPS_INSERTS)] = 0u; //low fps disabled
                    }

                    regChanges[static_cast<uint16_t> (curMBStart + MM_MB_CTRLSEQ)] =
                        static_cast<uint16_t> ( (mb.sequence.size() - 1) | (mb.safeForReconfig ? 0x8000 : 0));
                }

                //move up to the next measurement block register address
                curMBStart = static_cast<uint16_t> (curMBStart + AIO_MM_MB_OFFSET);
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
                       AIO_MM_MB_START,
                       AIO_MM_MB_OFFSET,
                       SEQIDXOFFSET,
                       AIO_SR_RECONFIGFLAGS,
                       AIO_MM_MAX_PARAMS,
                       AIO_MM_SR_PARAMSTART,
                       AIO_MM_SR_DECODESTART,
                       regChanges);
        }
    };

    class M2452_B1x_AIO_Strategy : public IImagerModeStrategy
    {
        const uint16_t AIO_EXPO = 0u;
        const uint16_t AIO_FR = 1u;
        const uint16_t AIO_PS = 2u;
        const uint16_t AIO_LPFSMFRATE = 0xB300;
        const uint16_t AIO_LOWFPS_COUNTERRELOAD = 0xB340;
        const uint16_t AIO_LOWFPS_INSERTS = 0xB341;
        const uint16_t AIO_START = 0xB301;
        const uint16_t SEQIDXOFFSET = 3u;

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
                    regChanges[static_cast<uint16_t> (AIO_START + AIO_EXPO + idx)] = seq.expo;
                    regChanges[static_cast<uint16_t> (AIO_START + AIO_PS + idx)] = static_cast<uint16_t> (seq.ps | seq.pllset << 9);

                    if (seq.fr_valEqZero)
                    {
                        regChanges[static_cast<uint16_t> (AIO_START + AIO_FR + idx)] = 0u;
                    }
                    else
                    {
                        regChanges[static_cast<uint16_t> (AIO_START + AIO_FR + idx)] = seq.fr;
                    }

                    //move up to the next LUT assignment register address
                    idx = static_cast<uint16_t> (idx + SEQIDXOFFSET);
                }
            }

            if (mbList.front().frameRateCounter > std::numeric_limits<uint16_t>::max())
            {
                //if the LPFSM framerate counter cannot handle the specified target framerate use the low-framerate firmware feature
                setLowFpsFrameRate (regChanges, mbList.front().frameRateCounter,
                                    AIO_LPFSMFRATE,
                                    AIO_LOWFPS_COUNTERRELOAD,
                                    AIO_LOWFPS_INSERTS);
            }
            else
            {
                regChanges[AIO_LPFSMFRATE] = static_cast<uint16_t> (mbList.front().frameRateCounter);
                regChanges[AIO_LOWFPS_INSERTS] = 0u; //low fps disabled
            }

            regChanges[CFGCNT_CTRLSEQ] = static_cast<uint16_t> ( (mbList.front().sequence.size() - 1) | 0x8000);
        }

        std::map < uint16_t, uint16_t > reconfigTranslation (const size_t mbCount, const std::map < uint16_t, uint16_t > &regChanges) const override
        {
            const uint16_t AIO_SR_NM_LPFSMFRATE = CFGCNT_S14_PS;
            const uint16_t AIO_SR_NM_LOWFPS_COUNTERRELOAD = CFGCNT_S15_EXPOTIME;
            const uint16_t AIO_SR_NM_LOWFPS_INSERTS = CFGCNT_S15_FRAMERATE;

            std::map <uint16_t, uint16_t> transformedRegisters;

            for (size_t sidx = AIO_START; sidx < AIO_CFGCNT_PLLCFG1_LUT1; sidx += SEQIDXOFFSET)
            {
                if (regChanges.count (static_cast<uint16_t> (AIO_EXPO + sidx)))
                {
                    transformedRegisters[static_cast<uint16_t> (AIO_NM_START + AIO_EXPO + ( (sidx - AIO_START) / SEQIDXOFFSET))] =
                        regChanges.at (static_cast<uint16_t> (AIO_EXPO + sidx));
                }

                if (regChanges.count (static_cast<uint16_t> (AIO_FR + sidx)))
                {
                    throw RuntimeError ("not supported by firmware");
                }
            }

            if (regChanges.count (AIO_LPFSMFRATE))
            {
                transformedRegisters[AIO_SR_NM_LPFSMFRATE] = regChanges.at (AIO_LPFSMFRATE);
            }

            if (regChanges.count (AIO_LOWFPS_COUNTERRELOAD))
            {
                transformedRegisters[AIO_SR_NM_LOWFPS_COUNTERRELOAD] = regChanges.at (AIO_LOWFPS_COUNTERRELOAD);
            }

            if (regChanges.count (AIO_LOWFPS_INSERTS))
            {
                transformedRegisters[AIO_SR_NM_LOWFPS_INSERTS] = regChanges.at (AIO_LOWFPS_INSERTS);
            }

            //it would be more clear to have this as separate write command, but it is also possible
            //to add this to the list of translated registers; the reason why it is added is that
            //in the mixed mode case it is required add it, so it is added here too for convenience
            transformedRegisters[AIO_SR_RECONFIGFLAGS] = 1u;

            return transformedRegisters;
        }

    };
}

ImagerM2452_B1x_AIO::ImagerM2452_B1x_AIO (const ImagerParameters &params) :
    ImagerM2452 (params, 1, 11),
    m_currentModeIsMixedMode (false),
    m_strategy (makeUnique<M2452_B1x_AIO_Strategy>())
{
    m_regReconfCnt = AIO_SR_RECONFIG_COUNTER;
}

ImagerM2452_B1x_AIO::~ImagerM2452_B1x_AIO()
{

}

std::unique_ptr<IPseudoDataInterpreter> ImagerM2452_B1x_AIO::createPseudoDataInterpreter()
{
    std::unique_ptr<IPseudoDataInterpreter> pseudoDataInter (new PseudoDataInterpreter_AIO);
    return pseudoDataInter;
}

void ImagerM2452_B1x_AIO::initialize()
{
    Imager::initialize();

    uint16_t regValue = 0;
    uint16_t regDs = 0;

    readImagerRegister (ANAIP_DESIGNSTEP, regDs);

    if (0x0B11 != regDs &&
            0x0B12 != regDs)
    {
        throw Exception ("wrong design step");
    }

    downloadInitialConfig (royale::imager::M2452_B1x::BaseConfig);

    loadFirmware (royale::imager::M2452_B1x::AIOFirmware_Version,
                  royale::imager::M2452_B1x::AIOFirmware_Page1);

    //activate AllInOne firmware (enables RAM FW execution)
    writeImagerRegister (ISM_CTRL, 2u);

    //copy ANAIP default values into FW RAM SSC area
    writeImagerRegister (AIO_SSC_INIT, 1u); // this is a write clear bit
    writeImagerRegister (m_regTrigger, m_valEndTrigger); //don't start capturing
    writeImagerRegister (ISM_EN, 1u); // <-- start AllInOne FW

    m_bridge->sleepFor (std::chrono::microseconds (100));
    readImagerRegister (ISM_ISMSTATE, regValue);

    if (regValue & 1 << 14)
    {
        throw RuntimeError (MSG_ISMACTIVE);
    }

    m_imagerState = ImagerState::Ready;
}

void ImagerM2452_B1x_AIO::loadFirmware (const uint32_t version, const std::map<uint16_t, uint16_t> &page1)
{
    if (page1.empty())
    {
        throw RuntimeError ("firmware page1 must not be empty");
    }

    const uint32_t bytecode_version_msb = page1.at (ISM_FW_RAM_VERSION_MSB) & 0x7ff;
    const uint32_t bytecode_version_lsb = page1.at (ISM_FW_RAM_VERSION_LSB) & 0x7ff;

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

    writeImagerRegister (ISM_MEMPAGE, 4 << 5);

    //write page 1
    trackAndWriteRegisters (page1);

    writeImagerRegister (ISM_MEMPAGE, 0x0000);
}

void ImagerM2452_B1x_AIO::reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex)
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

void ImagerM2452_B1x_AIO::startCapture()
{
    Imager::startCapture();

    uint16_t regValue = 0;

    readImagerRegister (m_regStatus, regValue);

    if (! (regValue & 2))
    {
        logMessage (ImageSensorLogType::Comment, MSG_CAPTUREBUSY);
        throw LogicError (MSG_CAPTUREBUSY);
    }

    //starting the sequencer
    m_executingUcd = m_preparedUcd;

    const bool enabledMixedMode = m_executingUcd.getMixedModeEnabled();
    const bool enabledSSC = m_executingUcd.getSSCEnabled();

    //if a NTC temperature sensor is connected to the imager enable NTC readout.
    const uint16_t ntcEnableBit = m_imagerParams.ntcSensorUsed ? 0x01 : 0x0;

    //remember: other functions are enabled with the same write call (e.g. SSC+MM: (1 << 6) | (1 << 5))
    readImagerRegister (ISM_ISMSTATE, regValue);
    regValue = static_cast<uint16_t> (regValue & ~ ( (1 << 7) | (1 << 6) | (1 << 5) | (1 << 0)));
    const uint16_t enableSWTrigger = (1 << 7);
    auto sscBit = static_cast<uint16_t> ( (enabledSSC ? 1 : 0) << 6);
    auto mixedModeBit = static_cast<uint16_t> ( (enabledMixedMode ? 1 : 0) << 5);

    regValue |= enableSWTrigger | sscBit | mixedModeBit | ntcEnableBit;
    writeImagerRegister (ISM_ISMSTATE, static_cast<uint16_t> (regValue));
    writeImagerRegister (ISM_EN, 0x0001); // <-- start AllInOne FW

    //the firmware needs some time to start up
    m_bridge->sleepFor (std::chrono::microseconds (100));

    //DMEM read access is not safe (skip checking iSM state)
    //the following lines just document the normal way to check for successful firmware startup
    // but as this register is not safe to be read the sleep in the previous line is done
    //readImagerRegister (ISM_ISMSTATE, regValue);
    //if (regValue & 1 << 14)
    //{
    //    throw RuntimeError (MSG_ISMACTIVE);
    //}

    writeImagerRegister (m_regTrigger, m_valStartTrigger);

    // Wait for the firmware to update the status register
    // \todo ROYAL-2901 How long should this sleep be?
    m_bridge->sleepFor (std::chrono::microseconds (100));

    evaluatePostStartState();
}

void ImagerM2452_B1x_AIO::shutDownSequencer()
{
    commitOrRollbackShadowedRegisters (false);
    Imager::shutDownSequencer();

    writeImagerRegister (ISM_EN, 2u); // <-- stop AllInOne firmware
}

bool ImagerM2452_B1x_AIO::isValidExposureTime (bool enabledMixedMode, size_t overallRawFrameCount, uint32_t exposureTime, uint32_t modFreq)
{
    if (enabledMixedMode)
    {
        //assume worst case that is supported (no partial reconfig support > 16 params);
        //that is a change of exposure (x)or framerate for each raw frame plus two parameters
        //for the overall framerate - no ps/pll or CTRLSEQ change is supported;
        const size_t numParamCountMax = std::min (2 + (overallRawFrameCount * 2), (size_t) 16);
        const auto mixedModeReconfigWorstCase = static_cast<double> (numParamCountMax) * 7.;
        const auto minExpoMixedMode = static_cast<uint32_t> ( (m_imagerParams.ntcSensorUsed ? 88u : 21u) + mixedModeReconfigWorstCase);

        if (exposureTime < minExpoMixedMode)
        {
            return false;
        }
    }
    else
    {
        const uint32_t minExpoNormalModeNoNTC = 8u;
        const uint32_t minExpoNormalModeWithNTC = 75u;

        if (exposureTime < (m_imagerParams.ntcSensorUsed ? minExpoNormalModeWithNTC : minExpoNormalModeNoNTC))
        {
            return false;
        }
    }

    return ImagerM2452::isValidExposureTime (enabledMixedMode, overallRawFrameCount, exposureTime, modFreq);
}

ImagerVerificationStatus ImagerM2452_B1x_AIO::verifyFrameRateSettings (const ImagerUseCaseDefinition &useCase, bool rawFrameBasedFrameRate)
{
    //low fps check of ImagerM2452 can be omitted (because this imager is supporting it)
    return Imager::verifyFrameRateSettings (useCase, false);
}

ImagerVerificationStatus ImagerM2452_B1x_AIO::verifySSCSettings (const ImagerUseCaseDefinition &useCase)
{
    //Frequencies and general SSC parameter limits are validated with verifyModulationSettings.
    //At this place there could be a validation if the SSC parameters
    //are okay, however no formula is yet provided by R&D for such
    //a calculation, it is currently up to the UCD creator to use
    //meaningful values.

    return ImagerVerificationStatus::SUCCESS;
}

void ImagerM2452_B1x_AIO::prepareSSCSettings (const uint16_t lutIndex,
        const std::vector<uint16_t> &pllCfg,
        std::map < uint16_t, uint16_t > &regChanges)
{
    //configure the SSC feature of the AllInOne firmware for this LUT
    regChanges[static_cast<uint16_t> (PLLCFG4_SSC_LUTx + (PLLCFG_SSC_LUT_OFFSET * lutIndex))] = pllCfg.at (3);
    regChanges[static_cast<uint16_t> (PLLCFG5_SSC_LUTx + (PLLCFG_SSC_LUT_OFFSET * lutIndex))] = pllCfg.at (4);
    regChanges[static_cast<uint16_t> (PLLCFG6_SSC_LUTx + (PLLCFG_SSC_LUT_OFFSET * lutIndex))] = pllCfg.at (5);
    regChanges[static_cast<uint16_t> (PLLCFG7_SSC_LUTx + (PLLCFG_SSC_LUT_OFFSET * lutIndex))] = pllCfg.at (6);
    regChanges[static_cast<uint16_t> (PLLCFG8_SSC_LUTx + (PLLCFG_SSC_LUT_OFFSET * lutIndex))] = pllCfg.at (7);
}

std::vector<MeasurementBlock> ImagerM2452_B1x_AIO::createMeasurementBlockList (const ImagerUseCaseDefinition &useCase) const
{
    if (useCase.getMixedModeEnabled())
    {
        return std::vector<MeasurementBlock> (8, MeasurementBlock (5));
    }

    return std::vector<MeasurementBlock> (m_defaultMeasurementBlockCount, MeasurementBlock (m_defaultMeasurementBlockCapacity));
}

std::map < uint16_t, uint16_t > ImagerM2452_B1x_AIO::prepareUseCase (const ImagerUseCaseDefinition &useCase)
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
        * The mixed mode feature of the AllInOne firmware does not have a dedicated safe reconfig shadow area
        * for the PLL setting. This means that it is not possible to change modulation frequencies while capturing.
        * Be aware that disabling the mixed mode (enable interleaved bit = 0) would require to change this
        * address to 928d as the normal safe-reconfig firmware is copying these registers.
        */
        m_regPllLutLower = CFGCNT_PLLCFG1_LUT1;
    }
    else
    {
        m_regPllLutLower = AIO_CFGCNT_PLLCFG1_LUT1;
    }

    if (modeChanged)
    {
        if (newMode)
        {
            m_strategy = makeUnique<M2452_B1x_MM_Strategy>();
        }
        else
        {
            m_strategy = makeUnique<M2452_B1x_AIO_Strategy>();
        }

        //location of PLL LUTs changed, invalidate the assignment
        m_lutAssignment.clear();
    }

    m_regDownloaded.erase (CFGCNT_CTRLSEQ); //force write (firmware possibly changed the register value)

    //create map for staging all changes
    std::map < uint16_t, uint16_t > regChanges;
    prepareFrameRateSettings (useCase);
    prepareModulationSettings (useCase, regChanges);
    preparePsSettings (useCase);
    prepareExposureSettings (useCase);

    m_strategy->generateMeasurementBlockConfig (m_mbList, regChanges);

    uint16_t roiCMin, roiCMax, roiRMin, roiRMax;
    prepareROI (useCase, roiCMin, roiCMax, roiRMin, roiRMax);
    regChanges[CFGCNT_ROICOL] = static_cast<uint16_t> ( (roiCMax << 8) | roiCMin);
    regChanges[CFGCNT_ROIROW] = static_cast<uint16_t> ( (roiRMax << 8) | roiRMin);

    return regChanges;
}
