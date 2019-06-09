#include <ContextCameraDevice.hpp>

#include "catch.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <cinttypes>
#include <climits>
#include <cmath>

#ifndef PRId64
// In C99, PRId64 is expected to be provided by cinttypes
static_assert (sizeof (long long) * CHAR_BIT == 64, "No system-supplied 64-bit printf format, and the fallback isn't 64-bit");
#define PRId64 "lld"
#endif

#ifdef _WINDOWS
#include<Windows.h>
#else
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#endif

#include "Helper.hpp"

// disable deprecation warnings under Windows
#ifdef WIN32
#pragma warning(disable:4996)
#endif

#ifdef _WINDOWS
#include <direct.h>
static const std::string dirSep ("\\");
#define GETCWD_CMD _getcwd
#else
static const std::string dirSep ("/");
#define GETCWD_CMD getcwd
#endif

using namespace royale::bdd;
using namespace royale;
using namespace std;

static std::string getCurrentDirectory()
{
    char buffer[1024];
    if (GETCWD_CMD (buffer, sizeof (buffer)) != NULL)
    {
        return std::string (buffer);
    }
    return std::string();
}

ContextCameraDevice::ContextCameraDevice() :
    m_cameraDevice (nullptr),
    m_listenerDepth (nullptr),
    m_listenerRaw (nullptr),
    m_lastExposureTime (0),
    m_capMode (CaptureMode::DepthMode)
{
}

ContextCameraDevice::~ContextCameraDevice()
{
    if (m_cameraDevice != nullptr)
    {
        bool isCapturing;
        CHECKED_IF (CameraStatus::SUCCESS == m_cameraDevice->isCapturing (isCapturing))
        {
            CHECK (CameraStatus::SUCCESS == m_cameraDevice->stopCapture());
        }
        m_cameraDevice.reset();
    }

    m_listenerDepth.reset();
    m_listenerRaw.reset();
}

CameraStatus ContextCameraDevice::setPlaybackFile (const std::string &playbackFile)
{
    m_playbackFile = playbackFile;
    return CameraStatus::SUCCESS;
}

bool ContextCameraDevice::fileExists (const std::string &fileName)
{
    std::string recordFile (getCurrentDirectory() + dirSep + fileName);

    if (FILE *file = fopen (recordFile.c_str(), "r"))
    {
        fclose (file);
        return true;
    }
    else
    {
        return false;
    }
}

void ContextCameraDevice::setRecordFileName (const std::string &recordFilename)
{
    m_recordFileName = recordFilename;
}

std::string ContextCameraDevice::getRecordFileName()
{
    return m_recordFileName;
}

CameraStatus ContextCameraDevice::startRecording (const std::string &fileName)
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    setRecordFileName (fileName);

    std::string recordFile (getCurrentDirectory() + dirSep + fileName);
    return m_cameraDevice->startRecording (recordFile);
}

CameraStatus ContextCameraDevice::stopRecording()
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    return m_cameraDevice->stopRecording();
}

CameraStatus ContextCameraDevice::connect (royale::String activationCode)
{
    if (m_cameraDevice != nullptr)
    {
        return CameraStatus::LOGIC_ERROR;
    }

    CameraManager manager (activationCode);

    if (m_playbackFile.empty())
    {
#if defined(TARGET_PLATFORM_ANDROID)
        auto connectedCameras = manager.getConnectedCameraList (ANDROID_USB_DEVICE_FD);
#else
        auto connectedCameras = manager.getConnectedCameraList();
#endif
        REQUIRE (static_cast<unsigned int> (connectedCameras.size()) >= 1u);
        m_cameraDevice = manager.createCamera (connectedCameras[0]);
        m_serialNumber = connectedCameras[0].toStdString();
    }
    else
    {
#ifdef _WINDOWS
        std::string recordFile (getCurrentDirectory() + "\\" + m_playbackFile);
#else
        std::string recordFile (getCurrentDirectory() + "/" + m_playbackFile);
#endif

        m_cameraDevice = manager.createCamera (recordFile);
        if (m_cameraDevice)
        {
            std::cout << "Successfully loaded recording." << std::endl;
        }
        else
        {
            return CameraStatus::COULD_NOT_OPEN;
        }
        m_serialNumber = "";
    }

    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    // With recordings, the initialize() must be called before getCameraName().
    auto initializeStatus = m_cameraDevice->initialize();
    REQUIRE (initializeStatus == CameraStatus::SUCCESS);
    return CameraStatus::SUCCESS;
}

CameraStatus ContextCameraDevice::registerDataListener()
{
    if (m_cameraDevice != nullptr)
    {
        m_capMode = DepthMode;
        m_listenerDepth.reset (new MockDepthCaptureListener());
        return m_cameraDevice->registerDataListener (m_listenerDepth.get());
    }
    else
    {
        return CameraStatus::RUNTIME_ERROR;
    }
}

CameraStatus ContextCameraDevice::registerDataListenerExtended()
{
    if (m_cameraDevice != nullptr)
    {
        CameraStatus status;

        m_capMode = RawMode;
        m_listenerRaw.reset (new MockExtendedCaptureListener());

        status = m_cameraDevice->setCallbackData (CallbackData::Raw);
        if (status != CameraStatus::SUCCESS)
        {
            return status;
        }

        return m_cameraDevice->registerDataListenerExtended (m_listenerRaw.get());
    }
    else
    {
        return CameraStatus::RUNTIME_ERROR;
    }
}

int64_t ContextCameraDevice::startUpPeriod()
{
    if (m_capMode == RawMode)
    {
        return (m_listenerRaw->getTimeofFirstCallback() - m_startCaptureMark).msecs();
    }
    return (m_listenerDepth->getTimeofFirstCallback() - m_startCaptureMark).msecs();
}

std::string ContextCameraDevice::getLastUseCaseName()
{
    return m_lastUseCaseName;
}

royale::CameraStatus ContextCameraDevice::setUseCaseDefault()
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    royale::Vector<royale::String> useCases;
    auto status = m_cameraDevice->getUseCases (useCases);
    if (status != CameraStatus::SUCCESS)
    {
        return status;
    }
    status = setUseCase (useCases[0].toStdString());

    // currently PicoS and Evalboard are not officially supported, but we want the tests to pass
    if (status == CameraStatus::CALIBRATION_DATA_ERROR)
    {
        royale::String cameraName;
        auto nameStatus = m_cameraDevice->getCameraName (cameraName);
        if (nameStatus == CameraStatus::SUCCESS
                && (cameraName == "PICOS_STANDARD" || cameraName == "EVALBOARD_LED_STANDARD"))
        {
            return CameraStatus::SUCCESS;
        }
    }

    return status;
}

royale::CameraStatus ContextCameraDevice::getUseCaseFastest (royale::String &modeName)
{
    royale::Vector<royale::String> useCases;
    auto status = m_cameraDevice->getUseCases (useCases);

    if (status == CameraStatus::SUCCESS && !useCases.empty())
    {
        int fps_max = 0;
        royale::String fastestMode;

        for (const auto &ucName : useCases)
        {
            uint32_t nStreams;
            if ( (m_cameraDevice->getNumberOfStreams (ucName, nStreams) != CameraStatus::SUCCESS) || (nStreams != 1))
            {
                // Ignore mixed-mode usecases (and those we can't get the number of streams for)
                continue;
            }

            int sequence = 0;
            int fps = 0;
            if (sscanf (ucName.c_str(), "MODE_%d_%dFPS", &sequence, &fps) != 2)
            {
                // Can't parse the usecase name
                continue;
            }

            if (fps >= fps_max)
            {
                fps_max = fps;
                fastestMode = ucName;
            }
        }
        if (!fastestMode.empty())
        {
            modeName = fastestMode;
        }
        else
        {
            // Did not find anything. Either the usecases have weird names, or are all mixed-mode...
            status = CameraStatus::USECASE_NOT_SUPPORTED;
        }
    }
    return status;
}

royale::CameraStatus ContextCameraDevice::setUseCaseFastest()
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    royale::String modeName;
    auto status = getUseCaseFastest (modeName);
    if (status == CameraStatus::SUCCESS)
    {
        status = setUseCase (modeName.toStdString());
    }
    return status;
}

CameraStatus ContextCameraDevice::setUseCase (const std::string &modeName)
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    auto status = m_cameraDevice->setUseCase (String::fromStdString (modeName));
    m_lastUseCaseName = modeName;
    return status;
}

CameraStatus ContextCameraDevice::destroyCamera()
{
    CameraStatus status = m_cameraDevice->stopCapture();

    m_cameraDevice.reset();
    m_cameraDevice = nullptr;

    return status;
}

CameraStatus ContextCameraDevice::startCapture()
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    bool connected;
    auto status = m_cameraDevice->isConnected (connected);
    if (status != CameraStatus::SUCCESS)
    {
        return status;
    }
    if (!connected)
    {
        return CameraStatus::DISCONNECTED;
    }

    m_startCaptureMark.reset();
    status = m_cameraDevice->startCapture();
    m_startCaptureDuration = m_startCaptureMark.elapsed();
    return status;
}

CameraStatus ContextCameraDevice::stopCapture()
{
    if (m_cameraDevice == nullptr)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    bool connected;
    auto status = m_cameraDevice->isConnected (connected);
    if (status != CameraStatus::SUCCESS)
    {
        return status;
    }
    if (!connected)
    {
        return CameraStatus::DISCONNECTED;
    }

    m_stopCaptureMark.reset();
    status = m_cameraDevice->stopCapture();
    m_stopCaptureDuration = m_stopCaptureMark.elapsed();
    return status;
}

float ContextCameraDevice::getAverageFps() const
{
    if (m_capMode == RawMode)
    {
        return m_averageFPSRaw;
    }
    return m_averageFPSDepth;
}

CameraStatus ContextCameraDevice::singleShot()
{
    return CameraStatus::NOT_IMPLEMENTED;
}

bool ContextCameraDevice::isCapturing() const
{
    bool capturing;
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->isCapturing (capturing));
    return capturing;
}

bool ContextCameraDevice::isMixedMode (const std::string &modeName) const
{
    uint32_t nStreams;
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->getNumberOfStreams (modeName, nStreams));
    return nStreams > 1;
}

int32_t ContextCameraDevice::getDiffToLastCallback()
{
    if (m_capMode == RawMode)
    {
        return static_cast<int32_t> (chrono::duration_cast<chrono::milliseconds> (m_listenerRaw->getLastTimeStamp() - m_currentTimeMS).count());
    }
    return static_cast<int32_t> (chrono::duration_cast<chrono::milliseconds> (m_listenerDepth->getLastTimeStamp() - m_currentTimeMS).count());
}

int32_t ContextCameraDevice::getCallbackCounts() const
{
    if (m_capMode == RawMode)
    {
        return m_listenerRaw->getCounterCallbacks();
    }
    return m_listenerDepth->getCallBackCounter();
}

void ContextCameraDevice::measureAverageFrameRate (unsigned int seconds)
{
    if (m_capMode == RawMode)
    {
        auto &stats = m_listenerRaw->getFrameRateStats();
        m_averageFPSRaw = 0.0f;
        stats.resetStats();
        std::this_thread::sleep_for (std::chrono::seconds (seconds));
        stats.updateStats();
        m_averageFPSRaw = static_cast<float> (stats.avg90_fps);
    }
    else
    {
        auto &stats = m_listenerDepth->getFrameRateStats();
        m_averageFPSDepth = 0.0f;
        stats.resetStats();
        std::this_thread::sleep_for (std::chrono::seconds (seconds));
        stats.updateStats();
        m_averageFPSDepth = static_cast<float> (stats.avg90_fps);
    }
}

long ContextCameraDevice::getElapsedTimeMS() const
{
    return static_cast<long> (m_elapsedTimeMS);
}

CameraStatus ContextCameraDevice::startTimer()
{
    m_timerStart.reset();
    return CameraStatus::SUCCESS;
}

CameraStatus ContextCameraDevice::stopTimer()
{
    m_elapsedTimeMS = m_timerStart.elapsed();
    return CameraStatus::SUCCESS;
}

CameraStatus ContextCameraDevice::getCurrentTimeInMS()
{
    m_currentTimeMS = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch());
    return CameraStatus::SUCCESS;
}

royale::CameraStatus ContextCameraDevice::setExposureTime (uint32_t time)
{
    m_lastExposureTime = time;

    royale::common::RoyaleProfiler elapsedUS;
    auto value = CameraStatus::DEVICE_IS_BUSY;
    while (true)
    {
        value = m_cameraDevice->setExposureTime (time);
        if (value == CameraStatus::DEVICE_IS_BUSY)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        else
        {
            break;
        }
    }

    m_exposureTimeChangeDurations.push_back (elapsedUS.elapsed ("us"));
    LOG_OUTPUT ("Setting exposure time to %d - took %" PRId64 "ms\n", time, elapsedUS.elapsed());

    // Short pause to give the imager reconfiguration some time to settle
    std::this_thread::sleep_for (std::chrono::milliseconds (25)); // one frame @ 40 fps

    return value;
}

royale::CameraStatus ContextCameraDevice::setExposureMode (ExposureMode exposureMode)
{
    m_lastExposureMode = exposureMode;

    royale::common::RoyaleProfiler elapsedUS;
    CameraStatus value = m_cameraDevice->setExposureMode (exposureMode);
    m_exposureModeChangeDurations.push_back (elapsedUS.elapsed ("us"));
    LOG_OUTPUT ("Setting exposure mode to %s - took %" PRId64 "ms\n", (exposureMode == ExposureMode::AUTOMATIC ? "AUTOMATIC" : "MANUAL"), elapsedUS.elapsed());
    return value;
}

uint32_t ContextCameraDevice::getLastExposureTime() const
{
    return m_lastExposureTime;
}

royale::CameraStatus ContextCameraDevice::checkDepthData()
{
    uint16_t width, height;
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->getMaxSensorWidth (width));
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->getMaxSensorHeight (height));

    REQUIRE (m_listenerDepth->getHeight() == height);
    REQUIRE (m_listenerDepth->getWidth() == width);
    REQUIRE (m_listenerDepth->getPointCount() == static_cast<size_t> (width * height));
    return CameraStatus::SUCCESS;
}

void ContextCameraDevice::diffBetweenUseCaseSwitch (int32_t timeDiff) const
{
    for (auto &i : m_operationModeChangeDurations)
    {
        REQUIRE (i < timeDiff);
    }
    return;
}

void ContextCameraDevice::diffBetweenExposureModeSwitch (int32_t timeDiffMicroSeconds) const
{
    for (auto &i : m_exposureModeChangeDurations)
    {
        REQUIRE (i < timeDiffMicroSeconds);
    }
    return;
}

void ContextCameraDevice::diffBetweenExposureTimeSwitch (int32_t timeDiffMicroSeconds) const
{
    for (auto &i : m_exposureTimeChangeDurations)
    {
        REQUIRE (i < timeDiffMicroSeconds);
    }
    return;
}

int64_t ContextCameraDevice::getStartCaptureDuration()
{
    return m_startCaptureDuration;
}

int64_t ContextCameraDevice::getStopCaptureDuration()
{
    return m_stopCaptureDuration;
}

royale::CameraStatus ContextCameraDevice::sweepAllUseCasesAndCalcFPS (MeasureMode measureMode, unsigned int time)
{
    m_operationModeChangeDurations.clear();

    if (m_capMode == RawMode)
    {
        m_measuredFPSRaw.clear();
    }
    else
    {
        m_measuredFPSDepth.clear();
    }

    bool capturing;
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->isCapturing (capturing));
    REQUIRE (capturing == true);

    royale::Vector<royale::String> useCases;
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->getUseCases (useCases));
    if (m_capMode != RawMode)
    {
        REQUIRE (useCases.size() >= 1u);
    }

    if (m_capMode == RawMode)
    {
        REQUIRE (m_listenerRaw != nullptr);
    }
    else
    {
        REQUIRE (m_listenerDepth != nullptr);
    }

    for (auto &mode : useCases)
    {
        // Check for and avoid mixed mode usecases
        if (isMixedMode (mode.c_str()))
        {
            LOG_OUTPUT ("Skipping operation mode %s (mixed mode)\n", mode.c_str());
            continue;
        }

        LOG_OUTPUT ("Setting operation mode %s", mode.c_str());

        // PicoS and EvalBoard are returning module_not_supported
        royale::common::RoyaleProfiler elapsedMS;
        auto status = m_cameraDevice->setUseCase (mode);
        m_operationModeChangeDurations.push_back (elapsedMS.elapsed());

        REQUIRE (status == CameraStatus::SUCCESS);

        // make a save delay in order to settle down new setting
        std::this_thread::sleep_for (std::chrono::seconds (2));

        float multiplier = 1.0;
        if (measureMode == MeasureMode::Equivalent)
        {
            int sequence_dummy, fps, fps_max, exposure_dummy;
            String fastestOpMode;
            REQUIRE (CameraStatus::SUCCESS == getUseCaseFastest (fastestOpMode));
            auto nItemsScanned = sscanf (fastestOpMode.c_str(), "MODE_%d_%dFPS_%d", &sequence_dummy, &fps_max, &exposure_dummy);
            REQUIRE (nItemsScanned == 3); // may fail for usecases with unusual names
            nItemsScanned = sscanf (mode.c_str(), "MODE_%d_%dFPS_%d", &sequence_dummy, &fps, &exposure_dummy);
            REQUIRE (nItemsScanned == 3); // may fail for usecases with unusual names

            multiplier = static_cast<float> (fps_max) / static_cast<float> (fps);
        }

        int32_t relevantTime = static_cast<int32_t> (std::ceil (multiplier * static_cast<float> (time)));

        if (time != 0)
        {
            if (m_capMode == RawMode)
            {
                measureAverageFrameRate (relevantTime);
                float fps = getAverageFps();
                LOG_OUTPUT (" - MEASURED %.2f FPS\n", fps);
                fflush (stdout);
                m_measuredFPSRaw[mode.toStdString()] = fps;
            }
            else
            {
                measureAverageFrameRate (relevantTime);
                float fps = getAverageFps();
                LOG_OUTPUT (" - MEASURED %.2f FPS\n", fps);
                fflush (stdout);
                m_measuredFPSDepth[mode.toStdString()] = fps;
            }
        }
        else
        {
            LOG_OUTPUT (" - took %" PRId64 " ms\n", m_operationModeChangeDurations.back());
        }
    }
    return CameraStatus::SUCCESS;
}

royale::CameraStatus ContextCameraDevice::checkCorrectFPSForAllUseCases (unsigned int range)
{
    std::map<std::string, float>::iterator iter;
    std::map<std::string, float>::iterator iterEnd;
    if (m_capMode == RawMode)
    {
        iter = m_measuredFPSRaw.begin();
        iterEnd = m_measuredFPSRaw.end();
    }
    else
    {
        iter = m_measuredFPSDepth.begin();
        iterEnd = m_measuredFPSDepth.end();
    }
    int sequence, fps, exposure;
    while (iter != iterEnd)
    {
        auto nItemsScanned = sscanf (iter->first.c_str(), "MODE_%d_%dFPS_%d", &sequence, &fps, &exposure);
        REQUIRE (nItemsScanned == 3); // may fail for usecases with unusual names
        LOG_OUTPUT ("%s - EXPECTED %2d vs MEASURED %.2f FPS\n",  iter->first.c_str(), fps, iter->second);
        fflush (stdout);
        REQUIRE (iter->second >= static_cast<float> (fps - range));
        REQUIRE (iter->second <= static_cast<float> (fps + range));
        ++iter;
    }
    return CameraStatus::SUCCESS;
}

void ContextCameraDevice::getExposureLimits (unsigned int &maxValue, unsigned int &minValue)
{

    royale::Pair <uint32_t, uint32_t> lim;
    REQUIRE (CameraStatus::SUCCESS == m_cameraDevice->getExposureLimits (lim));
    maxValue = lim.second;
    minValue = lim.first;
}
