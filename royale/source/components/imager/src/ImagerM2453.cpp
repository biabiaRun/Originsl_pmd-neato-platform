/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerM2453.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2453/PseudoDataInterpreter.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <thread>

using namespace std;
using namespace royale::imager;
using namespace royale::common;

namespace
{
    /**
    * Hardware limit for payload, in 8-bit bytes, please refer to Jama item MIB-SET-348
    * for detailed information. At the time of implementing the code using
    * the numBytesPerTransfer constant the Jama content was:
    *
    * The start address of the selected memory section has to be written to the SPI_WR_ADDR register.
    * Also the  length of the data packet has to be specified in the register SPI_LEN.
    * The maximum payload per SPI write is 256 Bytes. This means that SPI_LEN can be Maximum 259 (= 260 -1):
    * 1 Byte: command
    * 3 Bytes: address
    * 256 Bytes: payload
    */
    const std::size_t numBytesPerTransfer = 256;

    /** Hardware limit for payload, in 16-bit registers */
    const std::size_t maxPayloadSize = 128;
    const std::size_t regSize = 2;
    static_assert (numBytesPerTransfer == maxPayloadSize *regSize,
                   "payload size in bytes doesn't match payload size in registers");

    /**
    * Duration of the unconditional sleep after setting SPITRIG, before first polling for the
    * status.  The value 150us is chosen as the worst case value for the transfer of the command,
    * address and one full payload of numBytesPerTransfer bytes over the imager's SPI bus assuming
    * the System Clock is 160MHz, and that the SPI is configured to run at sys_clock/8.
    */
    const auto DEFAULT_TIME_BLOCK_TRANSFER = std::chrono::microseconds (150);

    /**
    * The conditional sleep between retries of reading the status.
    */
    const auto POLLING_INTERVAL = std::chrono::milliseconds (10);
}

ImagerM2453::ImagerM2453 (const ImagerParameters &params) :
    m_bridge {params.bridge},
    m_registerAccess {common::makeUnique<ImagerRegisterAccess> (params.bridge) },
    m_externalConfig {params.externalConfig},
    m_systemFrequency {params.systemFrequency},
    m_executingUseCaseIdentifier {},
    m_cachedConfigChangeCounter {0},
    m_cachedUseCaseChangeCounter {0},
    m_usesInternalCurrentMonitor {params.usesInternalCurrentMonitor},
    m_lastStopTime{}
{
    if (m_bridge == nullptr)
    {
        throw LogicError ("nullref exception");
    }

    if (!m_externalConfig)
    {
        throw RuntimeError ("this imager needs to have an external configuration provided");
    }

    if (!m_externalConfig->getUseCaseList().size())
    {
        throw RuntimeError ("the external configuration needs to contain at least one use case");
    }

    m_imagerState = ImagerState::Virgin;
}

bool ImagerM2453::configChangePending()
{
    uint16_t flags;
    m_bridge->readImagerRegister (M2453::CFGCNT_FLAGS, flags);

    // Bit 0: config_changed.
    // Bit 1: use_case_changed
    return (flags & 3u) != 0u;
}

// This should only be called when configChangePending() returned false and the imager is capturing
uint16_t ImagerM2453::triggerConfigChange()
{
    m_bridge->writeImagerRegister (M2453::CFGCNT_FLAGS, 0x0001u); // config_changed

    auto prevConfigChangedCounter = m_cachedConfigChangeCounter++;
    m_cachedConfigChangeCounter &= 0x0FFFu; // 12 bits

    return prevConfigChangedCounter;
}

// This should only be called when configChangePending() returned false and the imager is capturing
uint16_t ImagerM2453::triggerUseCaseChange()
{
    m_bridge->writeImagerRegister (M2453::CFGCNT_FLAGS, 0x0002u); // use_case_changed

    auto prevConfigChangedCounter = m_cachedUseCaseChangeCounter++;
    m_cachedUseCaseChangeCounter &= 0x0FFFu; // 12 bits

    return prevConfigChangedCounter;
}

void ImagerM2453::initialize()
{
    if (ImagerState::PowerUp != m_imagerState)
    {
        throw WrongState();
    }

    m_registerAccess->transferRegisterMapAuto (m_externalConfig->getInitializationMap());

    //transfer the firmware (no-op if no firmware provided)
    m_registerAccess->transferRegisterMapAuto (m_externalConfig->getFirmwarePage1());
    m_registerAccess->transferRegisterMapAuto (m_externalConfig->getFirmwarePage2());
    m_registerAccess->transferRegisterMapAuto (m_externalConfig->getFirmwareStartMap());

    m_imagerState = ImagerState::Ready;
}

string ImagerM2453::getSerialNumber()
{
    //read efuses and create an unique string that identifies the sensor
    std::vector < uint16_t > vals = getSerialRegisters();

    return ImagerM2453Serial (vals).toString();
}

void ImagerM2453::wake()
{
    if (ImagerState::PowerDown != m_imagerState)
    {
        throw WrongState();
    }

    this_thread::sleep_for (chrono::microseconds (1));
    m_bridge->setImagerReset (false);

    m_imagerState = ImagerState::PowerUp;
}

void ImagerM2453::sleep()
{
    if (ImagerState::Virgin != m_imagerState &&
            ImagerState::PowerUp != m_imagerState &&
            ImagerState::Ready != m_imagerState)
    {
        throw WrongState();
    }

    //set reset line to power down the imager
    m_bridge->setImagerReset (true);

    m_cachedConfigChangeCounter = 0u;
    m_cachedUseCaseChangeCounter = 0u;

    m_imagerState = ImagerState::PowerDown;
    m_lastStopTime = WaitClockType::now();
}

void ImagerM2453::startCapture()
{
    if (ImagerState::Ready != m_imagerState)
    {
        throw WrongState();
    }

    const auto &useCaseData = getUseCaseDataByIdentifier (m_executingUseCaseIdentifier);
    auto waitUntil = m_lastStopTime + useCaseData.waitTime;
    auto now = WaitClockType::now();
    if (waitUntil > now)
    {
        m_bridge->sleepFor (std::chrono::duration_cast<std::chrono::microseconds> (waitUntil - now));
    }

    m_registerAccess->transferRegisterMapAuto (m_externalConfig->getStartMap());

    m_imagerState = ImagerState::Capturing;
}

void ImagerM2453::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }

    const auto &useCaseData = getUseCaseDataByIdentifier (m_executingUseCaseIdentifier);
    if (useCaseData.modulationFrequencies.size() > M2453::nSequenceEntries)
    {
        throw LogicError ("Imager config has too many sequence entries!");
    }
    if (exposureTimes.size() != useCaseData.modulationFrequencies.size())
    {
        throw invalid_argument ("Number of exposure times doesn't fit current usecase");
    }

    std::map < uint16_t, uint16_t > regMap;

    auto nExposureTimes = exposureTimes.size();
    const auto expoTimeStride = M2453::CFGCNT_S01_EXPOTIME - M2453::CFGCNT_S00_EXPOTIME;
    for (size_t idx = 0; idx < nExposureTimes; ++idx)
    {
        auto regVal = calcRegExposure (exposureTimes.at (idx), useCaseData.modulationFrequencies.at (idx));
        auto reg = static_cast<uint16_t> (M2453::CFGCNT_S00_EXPOTIME + idx * expoTimeStride);
        regMap[reg] = regVal;
    }

    // Prevent exposure changes if there is still a change pending.
    // (we might also block here)
    if (configChangePending())
    {
        throw RuntimeError ("Can't update exposure times while config change is still pending");
    }

    // Now actually write to the imager.
    for (const auto &r : regMap)
    {
        m_bridge->writeImagerRegister (r.first, r.second);
    }

    // and trigger the update.
    reconfigIndex = triggerConfigChange();
}

void ImagerM2453::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    throw NotImplemented();
}

uint16_t ImagerM2453::stopCapture()
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }

    m_registerAccess->transferRegisterMapAuto (m_externalConfig->getStopMap());

    m_imagerState = ImagerState::Ready;
    m_lastStopTime = WaitClockType::now();
    return 0u;
}

const IImagerExternalConfig::UseCaseData &ImagerM2453::getUseCaseDataByIdentifier (const ImagerUseCaseIdentifier &useCaseIdentifier) const
{
    for (const auto &useCaseData : m_externalConfig->getUseCaseList())
    {
        if (useCaseData.guid == useCaseIdentifier)
        {
            return useCaseData;
        }
    }

    throw invalid_argument ("A valid use case identifier must be provided to this function call");
}

ImagerVerificationStatus ImagerM2453::verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier)
{
    try
    {
        const auto &useCaseData = getUseCaseDataByIdentifier (useCaseIdentifier);

        if (useCaseData.sequentialRegisterHeader.flashConfigAddress >> 24)
        {
            return ImagerVerificationStatus::FLASH_CONFIG;
        }
    }
    catch (const invalid_argument &)
    {
        return ImagerVerificationStatus::USECASE_IDENTIFIER;
    }

    return ImagerVerificationStatus::SUCCESS;
}

void ImagerM2453::executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier)
{
    if (ImagerState::Ready != m_imagerState)
    {
        throw WrongState();
    }

    m_executingUseCaseIdentifier = ImagerUseCaseIdentifier{};

    const auto &useCaseData = getUseCaseDataByIdentifier (useCaseIdentifier);

    if (!useCaseData.sequentialRegisterHeader.empty())
    {
        doFlashToImagerUseCaseTransfer (useCaseData);
    }
    else
    {
        m_registerAccess->transferRegisterMapAuto (useCaseData.registerMap);
    }

    m_executingUseCaseIdentifier = useCaseData.guid;
}

void ImagerM2453::doFlashToImagerUseCaseTransfer (const IImagerExternalConfig::UseCaseData &useCaseData)
{
    const auto flashAddress = useCaseData.sequentialRegisterHeader.flashConfigAddress;
    const auto flashConfigSize = useCaseData.sequentialRegisterHeader.flashConfigSize;

    if (useCaseData.sequentialRegisterHeader.flashConfigSize % regSize)
    {
        throw LogicError ("Data size is not a multiple of the number of bytes per register");
    }
    if ( (useCaseData.sequentialRegisterHeader.imagerAddress != 0) &&
            (useCaseData.sequentialRegisterHeader.imagerAddress != M2453::CFGCNT))
    {
        // This would just need a change to doFlashToImagerBlockTransfer, but for now let's just
        // disallow it, as it's probably an error in the useCaseData.
        throw NotImplemented ("Loading a use case from SPI to an unexpected address");
    }
    auto registerCount = flashConfigSize / regSize;

    for (std::size_t i = 0u; i < registerCount; i += maxPayloadSize)
    {
        const auto blockSize = std::min (maxPayloadSize, registerCount - i);

        doFlashToImagerBlockTransfer (narrow_cast<uint32_t> (flashAddress + i * regSize),
                                      narrow_cast<uint16_t> (i),
                                      blockSize);
    }
}

void ImagerM2453::doFlashToImagerBlockTransfer (uint32_t flashAddress, uint16_t cfgcntOffset, size_t payloadSize)
{
    //spiLen == 0 means 1 byte, adding 3 to the payloadSize as there are two 16 bit words for command and address
    const auto spiLen = static_cast<uint16_t> ( (payloadSize << 1) + 3u);
    const auto readBufferAddress = M2453::CFGCNT;

    const auto spiEnable = 1u << 14;
    const auto spiClockDiv8 = 2u;
    m_bridge->writeImagerRegister (M2453::SPICFG, static_cast<uint16_t> (spiEnable | spiClockDiv8));

    //put the SPI query data into the pixel memory
    m_bridge->writeImagerRegister (M2453::SPIWRADDR, M2453::PIXMEM);
    m_bridge->writeImagerRegister (M2453::PIXMEM,
                                   static_cast<uint16_t> ( (0x03 << 8) | (flashAddress >> 16)));
    m_bridge->writeImagerRegister (static_cast<uint16_t> (M2453::PIXMEM + 1u),
                                   static_cast<uint16_t> (flashAddress & 0xFFFF));

    //data read from flash should be transferred to the configuration container
    m_bridge->writeImagerRegister (M2453::SPIRADDR, static_cast<uint16_t> (readBufferAddress + cfgcntOffset));
    m_bridge->writeImagerRegister (M2453::SPILEN, static_cast<uint16_t> ( (0x07 << 13) | spiLen));

    //initiate the transfer
    m_bridge->writeImagerRegister (M2453::SPITRIG, 2u);

    //wait for the transfer to finish
    m_registerAccess->pollUntil (M2453::SPISTATUS, 1u, DEFAULT_TIME_BLOCK_TRANSFER, POLLING_INTERVAL);
}

uint16_t ImagerM2453::calcRegExposure (uint32_t expoTime, uint32_t modfreq) const
{
    static const std::array<uint16_t, 4> prescaleMap = { { 1, 8, 32, 128 } };

    auto texpo = [ = ] (uint16_t preScaler)
    {
        return std::floor (static_cast<double> (expoTime) * static_cast<double> (modfreq) / static_cast<double> (1.0e6 * preScaler));
    };

    for (std::size_t preScaler = 0; preScaler < prescaleMap.size(); preScaler++)
    {
        if (texpo (prescaleMap[preScaler]) <= (double) 0x3FFF)
        {
            return static_cast<uint16_t> (static_cast<uint16_t> (texpo (prescaleMap[preScaler])) | preScaler << 14);
        }
    }

    // \todo JIRA-3090 this throw is unreachable, because the time was already checked in isValidExposureTime
    throw OutOfBounds ("Exposure time can not be represented to the imager");
}

vector<size_t> ImagerM2453::getMeasurementBlockSizes() const
{
    for (const auto &useCaseData : m_externalConfig->getUseCaseList())
    {
        if (useCaseData.guid == m_executingUseCaseIdentifier)
        {
            return{ useCaseData.imageStreamBlockSizes };
        }
    }

    throw LogicError ("No use case was executed before");
}

void ImagerM2453::setLoggingListener (IImageSensorLoggingListener *)
{
    throw NotImplemented();
}

void ImagerM2453::setExternalTrigger (bool)
{
    throw InvalidValue();
}
