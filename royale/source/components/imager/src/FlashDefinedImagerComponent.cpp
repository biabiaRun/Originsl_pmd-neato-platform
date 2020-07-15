#include <imager/FlashDefinedImagerComponent.hpp>

#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <imager/ImagerSimpleHexSerialNumber.hpp>

#include <common/MakeUnique.hpp>

#include <cmath>

using namespace std;
using namespace royale::imager;
using namespace royale::common;

FlashDefinedImagerComponent::FlashDefinedImagerComponent (const ImagerParameters &params) :
    m_bridge{ params.bridge },
    m_registerAccess{ common::makeUnique<ImagerRegisterAccess> (params.bridge) },
    m_externalConfig{ params.externalConfig },
    m_executingUseCaseIdentifier{},
    m_cachedConfigChangeCounter{ 0 },
    m_cachedUseCaseChangeCounter{ 0 },
    m_usesInternalCurrentMonitor{ params.usesInternalCurrentMonitor },
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

void FlashDefinedImagerComponent::initialize()
{
    uint16_t regDs = 0;
    DesignStepInfo dsInfo = getDesignStepInfo();
    m_bridge->readImagerRegister (dsInfo.ANAIP_DESIGNSTEP_Address, regDs);

    bool found = false;
    for (uint16_t ds : dsInfo.designSteps)
    {
        if (ds == regDs)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw Exception ("wrong design step");
    }

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

void FlashDefinedImagerComponent::wake()
{
    if (ImagerState::PowerDown != m_imagerState)
    {
        throw WrongState();
    }

    this_thread::sleep_for (chrono::microseconds (1));
    m_bridge->setImagerReset (false);

    m_imagerState = ImagerState::PowerUp;
}

void FlashDefinedImagerComponent::sleep()
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

void FlashDefinedImagerComponent::startCapture()
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

void FlashDefinedImagerComponent::reconfigureTargetFrameRate (uint16_t targetFrameRate, uint16_t &reconfigIndex)
{
    throw NotImplemented ("This is not supported for the ImagerM2453");
}

uint16_t FlashDefinedImagerComponent::stopCapture()
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

vector<size_t> FlashDefinedImagerComponent::getMeasurementBlockSizes() const
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

void FlashDefinedImagerComponent::setLoggingListener (IImageSensorLoggingListener *)
{
    throw NotImplemented();
}

void FlashDefinedImagerComponent::setExternalTrigger (bool)
{
    throw InvalidValue();
}

std::string FlashDefinedImagerComponent::getSerialNumber()
{
    //read efuses and create an unique string that identifies the sensor
    std::vector < uint16_t > vals = getSerialRegisters();
    return ImagerSimpleHexSerialNumber (vals).toString();
}

void FlashDefinedImagerComponent::reconfigureExposureTimes (const std::vector<uint32_t> &exposureTimes, uint16_t &reconfigIndex)
{
    if (ImagerState::Capturing != m_imagerState)
    {
        throw WrongState();
    }

    ExpoTimeRegInfo expoInfo = getExpoTimeRegInfo();

    const auto &useCaseData = getUseCaseDataByIdentifier (m_executingUseCaseIdentifier);
    if (useCaseData.modulationFrequencies.size() > expoInfo.nSequenceEntries)
    {
        throw LogicError ("Imager config has too many sequence entries!");
    }
    if (exposureTimes.size() != useCaseData.modulationFrequencies.size())
    {
        throw invalid_argument ("Number of exposure times doesn't fit current usecase");
    }

    std::map < uint16_t, uint16_t > regMap;

    auto nExposureTimes = exposureTimes.size();
    const auto expoTimeStride = expoInfo.CFGCNT_S01_EXPOTIME_Address - expoInfo.CFGCNT_S00_EXPOTIME_Address;
    for (size_t idx = 0; idx < nExposureTimes; ++idx)
    {
        auto regVal = calcRegExposure (exposureTimes.at (idx), useCaseData.modulationFrequencies.at (idx));
        auto reg = static_cast<uint16_t> (expoInfo.CFGCNT_S00_EXPOTIME_Address + idx * expoTimeStride);
        regMap[reg] = regVal;
    }

    // Prevent exposure changes if there is still a change pending.
    // (we might also block here)
    if (configChangePending (expoInfo.CFGCNT_FLAGS_Address))
    {
        throw RuntimeError ("Can't update exposure times while config change is still pending");
    }

    // Now actually write to the imager.
    for (const auto &r : regMap)
    {
        m_bridge->writeImagerRegister (r.first, r.second);
    }

    // and trigger the update.
    reconfigIndex = triggerConfigChange (expoInfo.CFGCNT_FLAGS_Address);
}

const IImagerExternalConfig::UseCaseData &FlashDefinedImagerComponent::getUseCaseDataByIdentifier (const ImagerUseCaseIdentifier &useCaseIdentifier) const
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

uint16_t FlashDefinedImagerComponent::calcRegExposure (uint32_t expoTime, uint32_t modfreq) const
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

bool FlashDefinedImagerComponent::configChangePending (uint16_t CFGCNT_FLAGS_Address)
{
    uint16_t flags;
    m_bridge->readImagerRegister (CFGCNT_FLAGS_Address, flags);

    // Bit 0: config_changed.
    // Bit 1: use_case_changed
    return (flags & 3u) != 0u;
}

// This should only be called when configChangePending() returned false and the imager is capturing
uint16_t FlashDefinedImagerComponent::triggerConfigChange (uint16_t CFGCNT_FLAGS_Address)
{
    m_bridge->writeImagerRegister (CFGCNT_FLAGS_Address, 0x0001u); // config_changed

    auto prevConfigChangedCounter = m_cachedConfigChangeCounter++;
    m_cachedConfigChangeCounter &= 0x0FFFu; // 12 bits

    return prevConfigChangedCounter;
}

// This should only be called when configChangePending() returned false and the imager is capturing
uint16_t FlashDefinedImagerComponent::triggerUseCaseChange (uint16_t CFGCNT_FLAGS_Address)
{
    m_bridge->writeImagerRegister (CFGCNT_FLAGS_Address, 0x0002u); // use_case_changed

    auto prevConfigChangedCounter = m_cachedUseCaseChangeCounter++;
    m_cachedUseCaseChangeCounter &= 0x0FFFu; // 12 bits

    return prevConfigChangedCounter;
}

ImagerVerificationStatus FlashDefinedImagerComponent::verifyUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier)
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

void FlashDefinedImagerComponent::executeUseCase (const ImagerUseCaseIdentifier &useCaseIdentifier)
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
