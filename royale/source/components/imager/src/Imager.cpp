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
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/exceptions/ValidButUnchanged.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/NarrowCast.hpp>

#include <imager/Imager.hpp>
#include <imager/ImagerUseCaseDefinitionUpdater.hpp>

#include <thread>
#include <set>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace royale::imager;
using namespace royale::common;

const std::string Imager::MSG_USECASEEXECUTION ("use case execution");
const std::string Imager::MSG_CAPTUREBUSY ("startCapture failed, mtcu is busy");
const std::string Imager::MSG_WRONGIF ("startCapture failed, wrong interface setting");
const std::string Imager::MSG_WRONGROI ("startCapture failed, wrong roi setting");
const std::string Imager::MSG_STARTMTCUIDLE ("startCapture failed, mtcu is idle");
const std::string Imager::MSG_STOPMTCUIDLE ("stopCapture failed, mtcu is idle");
const std::string Imager::MSG_CFGCONFLICT ("the register configuration conflicts with registers set by the imager class");

/**
 * This is the system clock that is used by this imager implementation. It is a constant system parameter defined
 * by the DPHY-PLL configuration. The system clock is 1/3 of the actual DPHY-PLL frequency of 400MHz.
 */
const double Imager::FSYSCLK = 133333333.3;

Imager::Imager (const ImagerParameters &params,
                uint16_t modPllLutCount,
                uint16_t lutIdxOffset,
                uint16_t clkDiv,
                size_t defaultMeasurementBlockCount,
                size_t defaultMeasurementBlockCapacity) :
    ImagerMeasurementBlockBase (params.bridge, defaultMeasurementBlockCount, defaultMeasurementBlockCapacity),
    m_imagerParams (params),
    m_preparedUcd (ImagerUseCaseDefinition()),
    m_executingUcd (ImagerUseCaseDefinition()),
    MODPLLLUTCOUNT (modPllLutCount),
    LUTIDXOFFSET (lutIdxOffset),
    CLKDIV (clkDiv),
    m_regTrigger (0),
    m_valStartTrigger (0),
    m_valEndTrigger (0),
    m_regStatus (0),
    m_statusIdle (0),
    m_regPllLutLower (0),
    m_regReconfCnt (0),
    m_rowLimitSensor (0),
    m_columnLimitSensor (0),
    m_currentTrigger (ImgTrigger::I2C),
    m_lastStopTime(),
    m_lastActiveTailTime (std::chrono::microseconds::zero()),
    m_lastExecutedTailTime (std::chrono::microseconds::zero())
{
    m_imagerState = ImagerState::Virgin;
}

Imager::~Imager()
{

}

void Imager::initialize()
{
    if (ImagerState::PowerUp != m_imagerState)
    {
        throw WrongState();
    }
}

void Imager::wake()
{
    if (ImagerState::PowerDown != m_imagerState)
    {
        throw WrongState();
    }

    m_regDownloaded.clear();
    m_lutAssignment.clear();

    //trigger a low pulse on the reset line
    //note: for the Enclustra firmware the release
    //of the signal is ignored, the complete reset
    //sequence is done by the firmware itself when
    //calling the command for the falling edge
    logMessage (ImageSensorLogType::Sleep, "1");
    m_bridge->sleepFor (std::chrono::microseconds (1));
    logMessage (ImageSensorLogType::Reset, "High");
    m_bridge->setImagerReset (false);
    m_imagerState = ImagerState::PowerUp;
}

void Imager::sleep()
{
    if (ImagerState::Virgin != m_imagerState &&
            ImagerState::PowerUp != m_imagerState &&
            ImagerState::Ready != m_imagerState)
    {
        throw WrongState();
    }

    //set reset line to power down the imager
    logMessage (ImageSensorLogType::Reset, "Low");
    m_bridge->setImagerReset (true);

    m_imagerState = ImagerState::PowerDown;
    m_lastStopTime = WaitClockType::now();
}

void Imager::startCapture()
{
    if (ImagerState::Ready != m_imagerState)
    {
        throw WrongState();
    }

    // Depending on the pause between frames of the different use cases
    // we have to insert a sleep to remain eye safe.
    // For different customers it might be needed to adapt the customOffset
    auto pauseUCSource = m_lastActiveTailTime;
    auto pauseUCDestination = m_lastExecutedTailTime;
    auto customOffset = std::chrono::microseconds::zero();

    auto waitUntil = m_lastStopTime + pauseUCDestination - pauseUCSource - customOffset;
    auto now = WaitClockType::now();
    if (waitUntil > now)
    {
        auto waitTime = std::chrono::duration_cast<std::chrono::microseconds> (waitUntil - now);
        m_bridge->sleepFor (waitTime);
    }

    m_lastActiveTailTime = m_lastExecutedTailTime;
}

void Imager::reconfigure (const ImagerUseCaseDefinition &useCase, uint16_t &reconfigIndex)
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }

    //in mixed-mode the measurement block repeats are not safe for reconfig
    //(not part of the shadow config) and in non-mixed-mode the same is valid for the CFGCNT_CTRLSEQ register
    if (m_executingUcd.getRawFrames().size() != useCase.getRawFrames().size())
    {
        throw RuntimeError ("reconfiguration failed, it is not allowed to change the sequence length");
    }

    //Reconfigure should only change exposure times or frame rates but shall never
    //write any PLL or PLL SSC register
    size_t index = 0;
    for (const auto rawFrame : useCase.getRawFrames())
    {
        if (rawFrame.modulationFrequency &&
                !m_lutAssignment.count (rawFrame.modulationFrequency))
        {
            throw RuntimeError ("reconfiguration failed, it is not allowed to set a new modulation PLL frequency");
        }

        if (m_executingUcd.getRawFrames().at (index).dutyCycle != rawFrame.dutyCycle)
        {
            throw RuntimeError ("reconfiguration failed, it is not allowed to change a duty cycle");
        }

        index++;
    }

    //we are safe to call the reconfigure...
}

void Imager::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{
    auto newUcd = ImagerUseCaseDefinitionUpdater (m_executingUcd); // copy UCD
    newUcd.setExposureTimes (exposureTimes);
    reconfigure (newUcd, reconfigIndex);
}

void Imager::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    auto newUcd = ImagerUseCaseDefinitionUpdater (m_executingUcd); // copy UCD
    newUcd.setTargetFrameRate (targetFrameRate);
    reconfigure (newUcd, reconfigIndex);
}

void Imager::stopCapture()
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }
}

void Imager::shutDownSequencer()
{
    writeImagerRegister (m_regTrigger, m_valEndTrigger);

    //the sleep time is the sequence duration in ms plus 10ms just in case
    const auto sleeptime = static_cast<uint32_t> (getMaxSafeReconfigTimeMilliseconds (m_executingUcd) + 10u);

    //ROYAL-1777 sleep for one full sequence to keep polling at a minimum,
    //but ensure that the sequencer must have stopped already
    m_bridge->sleepFor (std::chrono::microseconds (sleeptime * 1000));

    //poll once for stopped sequencer
    uint16_t regValue = 0u;
    readImagerRegister (m_regStatus, regValue);

    if ( (regValue & m_statusIdle) != m_statusIdle)
    {
        throw Timeout ("sequencer timeout", "stopping the sequencer not succeeded");
    }
    m_lastStopTime = WaitClockType::now();
}

void Imager::initialRegisterConfig (const std::map < uint16_t, uint16_t > &baseConfig)
{
    //write imager base config
    logMessage (ImageSensorLogType::Comment, "Start of imager specific base configuration");
    trackAndWriteRegisters (baseConfig);

    //write camera module base config
    logMessage (ImageSensorLogType::Comment, "Start of camera module specific base configuration");
    trackAndWriteRegisters (m_imagerParams.baseConfig);
}

uint16_t Imager::getFreeLutIndex()
{
    uint16_t freeLut = 0;

    std::vector<uint16_t> indices;

    for (auto freq : m_lutAssignment)
    {
        indices.push_back (freq.second);
    }

    std::sort (indices.begin(), indices.end());

    for (auto index : indices)
    {
        if (index == freeLut)
        {
            freeLut++;
        }
    }

    return freeLut;
}

void Imager::updateModulationAssigmentLUT (const ImagerUseCaseDefinition &useCase)
{
    uint16_t newFrequencyCount = 0;
    std::vector < uint32_t > unusedFrequecies{};
    std::set < uint32_t > currentFrequencies{};
    std::set < uint32_t > newFrequencies{};

    //go through all raw frames from the use case definition to collect the MODPLL frequencies in use
    for (const auto &rawFrames : useCase.getRawFrames())
    {
        currentFrequencies.insert (rawFrames.modulationFrequency);
    }

    //remove unused frequencies from the map
    for (auto prevFreq : m_lutAssignment)
    {
        if (!currentFrequencies.count (prevFreq.first))
        {
            unusedFrequecies.push_back (prevFreq.first);
        }
    }

    //count how many new frequencies we have
    for (const auto &rawFrames : useCase.getRawFrames())
    {
        uint32_t modFreq = rawFrames.modulationFrequency;

        if (ImagerRawFrame::MODFREQ_AUTO() == modFreq)
        {
            modFreq = getGrayscaleModulationFrequency();
        }

        if (0 == m_lutAssignment.count (modFreq) &&
                0 == newFrequencies.count (modFreq))
        {
            newFrequencyCount++;
            newFrequencies.insert (modFreq);
        }
    }

    //erase unused frequencies to make place for new ones
    for (auto unusedFreq : unusedFrequecies)
    {
        if (newFrequencyCount--)
        {
            m_lutAssignment.erase (unusedFreq);
        }
        else
        {
            break;
        }
    }
}

void Imager::prepareFrameRateSettings (const ImagerUseCaseDefinition &useCase)
{
    std::vector<size_t> idx (m_mbList.size(), 0);
    const auto &rfList = useCase.getRawFrames();
    const auto rawFrameRate = useCase.getRawFrameRate();

    //timings the raw frame sets should have (including the delays for
    // achieving the target framerates of all streams of the use case)
    const auto rfTargetTimes = generateRawFrameTimings (useCase);

    //timings the raw frame sets actually have
    // (this includes exposure, readout and raw frame rate as well)
    std::vector<double> rawFrameTime (rfTargetTimes.size(), 0.);

    const uint16_t rawFrameRateRegVal = rawFrameRate > 0 ? (uint16_t) ::round (FSYSCLK / (1024.0 * rawFrameRate)) : 0u;

    //go through all used raw frame sets and summarize their timings
    for (const auto rf_mapping : m_rfAssignment)
    {
        const auto &rf = rfList.at (rf_mapping.first);
        const auto mbIndex = m_rfAssignment[rf_mapping.first];

        m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).fr_valEqZero = false;
        m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).fr = rawFrameRateRegVal;

        if (rf.isEndOfLinkedRawFrames)
        {
            calcRawFrameRateTime (useCase,
                                  rf.exposureTime,
                                  rf.modulationFrequency,
                                  rf.isStartOfLinkedRawFrames, // \todo ROYAL-2456 needs to fix this
                                  rawFrameTime.at (rf_mapping.first));

            const auto tRfsDiff = rfTargetTimes[rf_mapping.first] - rawFrameTime.at (rf_mapping.first);
            m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).fr_valEqZero = (0u == rawFrameRate) && (tRfsDiff < (.1e-9));

            //adjustment for last raw frame time to match the raw frame set target
            m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).fr =
                (uint16_t) ::round (FSYSCLK / (1024.0 * (1.0 / rfTargetTimes[rf_mapping.first])));
        }

        idx[mbIndex]++;
    }

    for (MeasurementBlockId idx = 0; idx < m_mbList.size(); idx++)
    {
        double tTargetForMeasurementBlock = 0.;        //!< used to sum up the single raw frame set target times for the current MB
        double tLastRfsTargetForMeasurementBlock = 0.; //!< used to store the RFS target time of the last RFS of the current MB
        double tLastMeasurementSequence = 0.;          //!< used to store the RFS time of the last RFS of the current MB

        for (const auto rf_mapping : m_rfAssignment)
        {
            if (rf_mapping.second == idx)
            {
                tLastRfsTargetForMeasurementBlock = rfTargetTimes.at (rf_mapping.first);
                tTargetForMeasurementBlock = tTargetForMeasurementBlock + tLastRfsTargetForMeasurementBlock;
                tLastMeasurementSequence = rawFrameTime.at (rf_mapping.first);
            }
        }

        //get the time the complete measurement sequence of the MB needs, but without
        //the delay of the last raw frame of it
        const auto tMeasurementSequence = tTargetForMeasurementBlock - tLastRfsTargetForMeasurementBlock + tLastMeasurementSequence;

        //note: only target frame rate delays at the last active raw frame of a MB can
        //be optimized for low power (any raw frame rate of the other raw frames cannot
        //be used to enter the low power mode as the counter for this mode only starts to run
        //at the end of a MB) - the same is valid for imagers with only a single MB
        prepareMeasurementBlockTargetTime (idx, tTargetForMeasurementBlock, tMeasurementSequence);
    }
}

uint32_t Imager::getGrayscaleModulationFrequency() const
{
    //lowest frequency common for M245x derivates shall be used for grayscale
    //exposures to enable longest possible exposure times
    return 3152500;
}

void Imager::prepareExposureSettings (const ImagerUseCaseDefinition &useCase)
{
    std::vector<size_t> idx (m_mbList.size(), 0);
    const auto &rfList = useCase.getRawFrames();

    for (const auto rf_mapping : m_rfAssignment)
    {
        const auto &rf = rfList.at (rf_mapping.first);
        const auto mbIndex = m_rfAssignment[rf_mapping.first];
        uint32_t modfreq = rf.modulationFrequency;

        if (ImagerRawFrame::MODFREQ_AUTO() == modfreq)
        {
            modfreq = getGrayscaleModulationFrequency();
        }

        const auto expo = calcRegExposure (rf.exposureTime, modfreq);
        m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).expo = expo;
        idx[mbIndex]++;
    }
}

void Imager::prepareSSCSettings (const uint16_t lutIndex, const std::vector<uint16_t> &pllCfg, std::map < uint16_t, uint16_t > &regChanges)
{
    //only SSC specialized imagers will add register configurations for SSC
}

void Imager::prepareModulationSettings (const ImagerUseCaseDefinition &useCase, std::map < uint16_t, uint16_t > &regChanges)
{
    std::vector<uint16_t> pllcfg (8);
    updateModulationAssigmentLUT (useCase);

    //if no use case has yet been executed the default ctor of UseCaseDefinition will return false for getSSCEnabled()
    if (m_executingUcd.getSSCEnabled() != useCase.getSSCEnabled())
    {
        //The map stores only frequencies and not the SSC flag. As the SSC flag applies
        //to all entries of the LUT assignment, except the grayscale frequency, the calculation
        //must be re-run for all frequencies. Doing the grayscale frequency calculation is
        //superfluous but is not be optimized because switching between SSC and non-SSC use cases
        //is not expected to be used often.
        m_lutAssignment.clear();
    }

    //go through all raw frames from the use case definition to assign the appropriate LUT
    for (const auto &rawFrame : useCase.getRawFrames())
    {
        uint32_t modfreq = rawFrame.modulationFrequency;
        bool wavegenEnable = useCase.getSSCEnabled();

        if (ImagerRawFrame::MODFREQ_AUTO() == modfreq)
        {
            modfreq = getGrayscaleModulationFrequency();
        }

        if (rawFrame.grayscale)
        {
            wavegenEnable = false;
        }

        if (!m_lutAssignment.count (modfreq))
        {
            //retrieve a free lut index
            const uint16_t lutIndex = getFreeLutIndex();

            if (lutIndex == MODPLLLUTCOUNT)
            {
                throw OutOfBounds (MSG_USECASEEXECUTION, "failed to aquire a free LUT index");
            }

            //calculate and add new LUT entry to map (use CLK/4 divider)
            m_pllCalc->pllSettings (static_cast<double> (modfreq * CLKDIV), wavegenEnable, pllcfg,
                                    rawFrame.ssc_freq, rawFrame.ssc_kspread, rawFrame.ssc_delta);
            m_lutAssignment[modfreq] = lutIndex;

            //set the LUT registers to the new frequency
            regChanges[static_cast<uint16_t> (m_regPllLutLower + (LUTIDXOFFSET * lutIndex) + 0)] = pllcfg.at (0);
            regChanges[static_cast<uint16_t> (m_regPllLutLower + (LUTIDXOFFSET * lutIndex) + 1)] = pllcfg.at (1);
            regChanges[static_cast<uint16_t> (m_regPllLutLower + (LUTIDXOFFSET * lutIndex) + 2)] = pllcfg.at (2);

            if (wavegenEnable)
            {
                prepareSSCSettings (lutIndex, pllcfg, regChanges);
            }
        }
    }

    prepareModulationAssigment (useCase, regChanges);
}

void Imager::prepareModulationAssigment (const ImagerUseCaseDefinition &useCase, std::map < uint16_t, uint16_t > &regChanges)
{
    std::vector<size_t> idx (m_mbList.size(), 0);

    const auto &rfList = useCase.getRawFrames();

    for (const auto rf_mapping : m_rfAssignment)
    {
        const auto &rf = rfList.at (rf_mapping.first);
        const auto mbIndex = m_rfAssignment[rf_mapping.first];

        if (rf.modulationFrequency)
        {
            //assign the LUT to the raw frame entry
            m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).pllset = m_lutAssignment[rf.modulationFrequency];
        }
        else
        {
            m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).pllset = m_lutAssignment[getGrayscaleModulationFrequency()];
        }

        idx[mbIndex]++;
    }
}

void Imager::preparePsSettings (const ImagerUseCaseDefinition &useCase)
{
    std::vector<size_t> idx (m_mbList.size(), 0);

    const auto &rfList = useCase.getRawFrames();

    for (const auto rf_mapping : m_rfAssignment)
    {
        const auto &rf = rfList.at (rf_mapping.first);
        const auto mbIndex = m_rfAssignment[rf_mapping.first];

        const auto  dutyCycle = rf.dutyCycle == ImagerRawFrame::ImagerDutyCycle::DC_AUTO ?
                                m_imagerParams.dutyCycle : rf.dutyCycle;

        m_mbList.at (mbIndex).sequence.at (idx[mbIndex]).ps =
            static_cast<uint16_t> ( (rf.grayscale ? 0x2000 : 0x0000) | getPhaseMapping().at (dutyCycle).at (rf.phaseAngle));

        idx[mbIndex]++;
    }
}

void Imager::prepareROI (const ImagerUseCaseDefinition &useCase,
                         uint16_t &roiCMin,
                         uint16_t &roiCMax,
                         uint16_t &roiRMin,
                         uint16_t &roiRMax)
{
    useCase.getStartOfROI (roiCMin, roiRMin);
    useCase.getImage (roiCMax, roiRMax);

    //convert size value to position value
    roiRMax = narrow_cast<uint16_t> ( (roiRMax + roiRMin) - 1);
    roiCMax = narrow_cast<uint16_t> ( (roiCMax + roiCMin) - 1);

    adjustRowCount (roiRMax);

    LOG (INFO) << "[Imager] ROI setting: "
               << "CMin=" << roiCMin << " "
               << "CMax=" << roiCMax << " "
               << "RMin=" << roiRMin << " "
               << "RMax=" << roiRMax;
}

ImagerVerificationStatus Imager::verifyPhaseSettings (const ImagerUseCaseDefinition &useCase)
{
    for (const auto &rawFrame : useCase.getRawFrames())
    {
        auto dutyCycle = rawFrame.dutyCycle;
        if (dutyCycle == ImagerRawFrame::ImagerDutyCycle::DC_AUTO)
        {
            dutyCycle = m_imagerParams.dutyCycle;
        }
        if (dutyCycle == ImagerRawFrame::ImagerDutyCycle::DC_AUTO)
        {
            return ImagerVerificationStatus::DUTYCYCLE;
        }
        if (0 == getPhaseMapping().count (dutyCycle))
        {
            return ImagerVerificationStatus::DUTYCYCLE;
        }
    }

    return ImagerVerificationStatus::SUCCESS;
}

ImagerVerificationStatus Imager::verifyRegion (const ImagerUseCaseDefinition &useCase)
{
    uint16_t activeColumnCount, activeRowCount, activeRowOffset, activeColumnOffset;
    useCase.getImage (activeColumnCount, activeRowCount);
    useCase.getStartOfROI (activeColumnOffset, activeRowOffset);

    // adjust according to pseudodata placement
    adjustRowCount (activeRowCount);

    //first of all test if region exceeds the sensor area
    if (static_cast<uint16_t> (activeRowOffset + activeRowCount) > m_rowLimitSensor ||
            static_cast<uint16_t> (activeColumnOffset + activeColumnCount) > m_columnLimitSensor)
    {
        return ImagerVerificationStatus::REGION;
    }

    if (0 != activeColumnOffset % 16)
    {
        return ImagerVerificationStatus::REGION;
    }

    if (activeColumnCount < 32 || 0 != activeColumnCount % 16)
    {
        return ImagerVerificationStatus::REGION;
    }

    return ImagerVerificationStatus::SUCCESS;
}

ImagerVerificationStatus Imager::verifyFrameRateSettings (const ImagerUseCaseDefinition &useCase, bool rawFrameBasedFrameRate)
{
    if (0u == useCase.getTargetRate())
    {
        LOG (ERROR) << "It is not allowed to have no use case frame rate limitation";
        return ImagerVerificationStatus::FRAMERATE;
    }

    try
    {
        if (useCase.getTailTime().count() <= 0)
        {
            LOG (ERROR) << "getTailTime returned something non-positive (can't happen!)"; // it should throw instead
            return ImagerVerificationStatus::EXPOSURE_TIME;
        }
    }
    catch (const common::LogicError &)
    {
        LOG (ERROR) << "getTailTime failed, cumulated exposures are too long for frame rate";
        return ImagerVerificationStatus::FRAMERATE;
    }

    const auto &rfList = useCase.getRawFrames();
    const auto rfTargetTimes = generateRawFrameTimings (useCase);

    for (size_t idx = 0; idx < rfList.size(); idx++)
    {
        if (rfList[idx].tEyeSafety > 0.)
        {
            LOG (ERROR) << "Adding a delay for eye safety is not yet supported";
            return ImagerVerificationStatus::FRAMERATE;
        }

        double tRFS = 0.;

        if (rfTargetTimes[idx] < 0. ||
                !calcRawFrameRateTime (useCase, rfList[idx].exposureTime, rfList[idx].modulationFrequency, false, tRFS) ||
                tRFS > rfTargetTimes[idx])
        {
            LOG (ERROR) <<
                        "Raw frame does not fit into master interval or consumes more time than granted ==> invalid use case definition";
            return ImagerVerificationStatus::FRAMERATE;
        }
        else
        {
            //verify for a frame rate counter register value overflow
            if (rawFrameBasedFrameRate && (::round (FSYSCLK * rfTargetTimes[idx] / 1024.) > 65535.))
            {
                LOG (ERROR) << "Requested framerate of " <<
                            useCase.getTargetRate() << " Hz is too low (sleep time does not fit int 16 bit register)";
                return ImagerVerificationStatus::FRAMERATE;
            }
        }
    }

    return ImagerVerificationStatus::SUCCESS;
}

ImagerVerificationStatus Imager::verifyRawFrames (const ImagerUseCaseDefinition &useCase)
{
    if (!verifyRawFrameAssignment (useCase))
    {
        return ImagerVerificationStatus::SEQUENCER;
    }

    for (const auto &rfs : useCase.getRawFrames())
    {
        if (!isValidExposureTime (useCase.getMixedModeEnabled(),
                                  useCase.getRawFrames().size(),
                                  rfs.exposureTime,
                                  rfs.modulationFrequency))
        {
            return ImagerVerificationStatus::EXPOSURE_TIME;
        }
    }

    return ImagerVerificationStatus::SUCCESS;
}

ImagerVerificationStatus Imager::verifyModulationSettings (const ImagerUseCaseDefinition &useCase,
        const uint16_t clkdiv,
        const uint16_t modpllcount)
{
    std::vector<uint16_t> pllcfg;
    std::map < uint32_t, bool > lutAssigment;

    //go through all raw frames from the use case definition
    for (const auto &rawFrame : useCase.getRawFrames())
    {
        if (rawFrame.modulationFrequency)
        {
            if (rawFrame.modulationFrequency > m_imagerParams.maxModulationFrequency)
            {
                LOG (ERROR) << "Specified modulation frequency is higher than camera module is supporting";
                return ImagerVerificationStatus::MODULATION_FREQUENCY;
            }

            //verify if modulation frequency is a tested one
            if (rawFrame.modulationFrequency % 10000 != 0)
            {
                return ImagerVerificationStatus::MODULATION_FREQUENCY;
            }

            if (!m_pllCalc->pllSettings (static_cast<double> (rawFrame.modulationFrequency * clkdiv), false, pllcfg))
            {
                return ImagerVerificationStatus::MODULATION_FREQUENCY;
            }

            lutAssigment[rawFrame.modulationFrequency] = true;
        }
        else
        {
            //if no modulation frequency is specified the raw frames must not be modulated
            if (!rawFrame.grayscale)
            {
                return ImagerVerificationStatus::MODULATION_FREQUENCY;
            }

            lutAssigment[getGrayscaleModulationFrequency()] = true;
        }
    }

    if (lutAssigment.size() > modpllcount || 0 == lutAssigment.size())
    {
        return ImagerVerificationStatus::MODULATION_FREQUENCY;
    }

    return ImagerVerificationStatus::SUCCESS;
}

ImagerVerificationStatus Imager::verifySSCSettings (const ImagerUseCaseDefinition &useCase)
{
    //by default an imager does not support the SSC feature
    if (useCase.getSSCEnabled())
    {
        return ImagerVerificationStatus::MODULATION_FREQUENCY;
    }

    return ImagerVerificationStatus::SUCCESS;
}

ImagerVerificationStatus Imager::verifyUseCase (const ImagerUseCaseDefinition &useCase)
{
    ImagerVerificationStatus status;

    if ( (status = verifyRawFrames (useCase)) != ImagerVerificationStatus::SUCCESS)
    {
        return status;
    }

    if ( (status = verifyRegion (useCase)) != ImagerVerificationStatus::SUCCESS)
    {
        return status;
    }

    if ( (status = verifyModulationSettings (useCase)) != ImagerVerificationStatus::SUCCESS)
    {
        return status;
    }

    if ( (status = verifySSCSettings (useCase)) != ImagerVerificationStatus::SUCCESS)
    {
        return status;
    }

    if ( (status = verifyFrameRateSettings (useCase)) != ImagerVerificationStatus::SUCCESS)
    {
        return status;
    }

    if ( (status = verifyPhaseSettings (useCase)) != ImagerVerificationStatus::SUCCESS)
    {
        return status;
    }

    return ImagerVerificationStatus::SUCCESS;
}

void Imager::executeUseCase (const ImagerUseCaseDefinition &useCase)
{
    if (ImagerState::Ready != m_imagerState)
    {
        throw WrongState();
    }

    //only a single use case is supported by this implementation
    // no need to reconfigure anything, base configuration done already at initialization
    if (verifyUseCase (useCase) != ImagerVerificationStatus::SUCCESS)
    {
        throw Exception ("use case not supported");
    }

    //the imager's sequencer layout can change depending on the use case
    m_mbList = createMeasurementBlockList (useCase);

    //assign raw frame sets to measurement blocks
    generateRawFrameAssignment (useCase);

    //remember the original configuration for recovering
    std::map < uint16_t, uint16_t > regBase = m_regDownloaded;

    //use concrete imager implementation to gather the config from the use case definition
    // and resolve it by comparision with the current configuration of the hardware
    std::map < uint16_t, uint16_t > regChanges = resolveConfiguration (prepareUseCase (useCase));

    try
    {
        //this tracks all register changes use the m_regDownloaded map
        trackAndWriteRegisters (regChanges);

        //remember the use case that will become active after the next start start of the sequencer
        m_preparedUcd = useCase;
        m_lastExecutedTailTime = m_preparedUcd.getTailTime();
    }
    catch (const Exception &) //catch all exceptions thrown by bridge when HW access fails
    {
        //something went wrong, hardware might be in inconsistent state, try to recover

        //re-initialize the imager
        sleep();
        wake();
        initialize();

        //download latest working configuration
        trackAndWriteRegisters (regBase);

        //throw execute use case failure exception
        throw ValidButUnchanged();
    }
}

void Imager::setExternalTrigger (bool useExternalTrigger)
{
    if (m_imagerState != ImagerState::PowerDown &&
            m_imagerState != ImagerState::PowerUp &&
            m_imagerState != ImagerState::Virgin)
    {
        throw WrongState ("Wrong imager state to change the external trigger");
    }

    if (useExternalTrigger)
    {
        if (m_imagerParams.externalTrigger == ImgTrigger::I2C)
        {
            throw InvalidValue ("Module doesn't support external triggering");
        }
        else
        {
            m_currentTrigger = m_imagerParams.externalTrigger;
        }
    }
    else
    {
        m_currentTrigger = ImgTrigger::I2C;
    }
}
