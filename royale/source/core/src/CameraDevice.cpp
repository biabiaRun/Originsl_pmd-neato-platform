/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <device/CameraDevice.hpp>
#include <common/exceptions/ResourceError.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/RoyaleLogger.hpp>
#include <common/FileSystem.hpp>
#include <common/NarrowCast.hpp>

#include <common/exceptions/APIExceptionHandling.hpp>

#include <usecase/UseCaseSlave.hpp>

#include <algorithm>
#include <iostream>
#include <chrono>

using namespace royale;
using namespace royale::processing;
using namespace royale::common;
using namespace royale::device;
using namespace royale::config;
using namespace royale::usecase;

CameraDevice::CameraDevice (
    CameraAccessLevel level,
    const std::string &id,
    std::unique_ptr<CameraCore> module,
    std::shared_ptr<const ICoreConfig> config,
    std::shared_ptr<IProcessing> processing,
    CallbackData cbData) :
    CameraDeviceBase (level, id, config->getCameraName().toStdString(),
                      processing, cbData),
    m_cameraCore (std::move (module)),
    m_config (config),
    m_lensCenter (0, 0),
    m_useExternalTrigger (false)
{
    m_cameraCore->setCaptureListener (m_processing.get());
}

CameraDevice::~CameraDevice()
{
    if (m_recording && m_recording->isRecording())
    {
        auto status = stopRecording();
        if (status != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras stopRecording method failed inside its destructor. CameraStatus = \"" << royale::getStatusString (status) << "\"";
        }
    }
    if (m_processing)
    {
        royale::processing::DataListeners defaultListeners;
        m_processing->registerDataListeners (defaultListeners);

        m_processing->unregisterExposureListener();
    }
    m_cameraCore.reset(); // ensure CameraCore gets destroyed before processing
}

void CameraDevice::setRecordingEngine (std::unique_ptr<IRecord> recording)
{
    m_recording = std::move (recording);

    auto status = updateRecordingEngineListener();
    if (status != CameraStatus::SUCCESS)
    {
        LOG (ERROR) << "Cameras updateRecordingEngineListener method failed inside its setRecordingEngine method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
    }
}

void CameraDevice::updateSupportedUseCases()
{
    m_supportedUseCaseNames.clear();

    switch ( (CallbackData) m_callbackData)
    {
        case CallbackData::Raw:
            {
                for (auto &uc : m_availableUseCases)
                {
                    if (m_accessLevel >= uc.getAccessLevel())
                    {
                        if (m_cameraCore->verifyUseCase (uc.getDefinition()) == VerificationStatus::SUCCESS)
                        {
                            m_supportedUseCaseNames.push_back (uc.getName());
                        }
                    }
                }
                break;
            }
        case CallbackData::Depth:
        case CallbackData::Intermediate:
            {
                for (auto &uc : m_availableUseCases)
                {
                    if (m_accessLevel >= uc.getAccessLevel() &&
                            (uc.getCallbackData() == CallbackData::Depth ||
                             uc.getCallbackData() == CallbackData::Intermediate))
                    {
                        if (m_cameraCore->verifyUseCase (uc.getDefinition()) == VerificationStatus::SUCCESS)
                        {
                            m_supportedUseCaseNames.push_back (uc.getName());
                        }
                    }
                }
                break;
            }
        default:
            break;
    }
}

CameraStatus CameraDevice::updateExposureTimes (const Vector<uint32_t> &exposureTimes)
{
    std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);
    auto savedExposureTimes = m_currentUseCaseDefinition->getExposureTimes();
    try
    {
        m_currentUseCaseDefinition->setExposureTimes (exposureTimes);

        if (isCapturing())
        {
            if (m_useExternalTrigger)
            {
                // If we're using an external trigger the reconfig counter will
                // never be incremented, because it is reset for a new trigger.
                // That's why we have to use a different way to reconfigure the
                // exposure
                m_cameraCore->stopCapture();
                m_cameraCore->setUseCase (m_currentUseCaseDefinition);
                m_cameraCore->startCapture();
            }
            else
            {
                m_cameraCore->reconfigureImagerExposureTimes (m_currentUseCaseDefinition, m_currentUseCaseDefinition->getExposureTimes().toStdVector());
            }
        }
        else
        {
            if (activateUseCase() != CameraStatus::SUCCESS)
            {
                throw RuntimeError ("Error activating the use case");
            }
        }

    }
    catch (const Exception &e)
    {
        m_currentUseCaseDefinition->setExposureTimes (savedExposureTimes);

        if (e.getStatus() == CameraStatus::DEVICE_IS_BUSY)
        {
            LOG (ERROR) << "CameraDevice::updateExposureTimes(): camera busy while setting the new exposure values";
        }
        else
        {
            LOG (ERROR) << "CameraDevice::updateExposureTimes(): an exception was thrown while setting the new exposure values: " << e.what();
        }
        return e.getStatus();
    }
    return CameraStatus::SUCCESS;
}

// ----------------------------------------------------------------------------------------------
//                             ICameraDevice API implementation
// ----------------------------------------------------------------------------------------------

CameraStatus CameraDevice::initialize() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    // If device is already initialized, skip second initialization to avoid unknown behavior
    if (m_isPartiallyInitialized || m_isInitialized)
    {
        return CameraStatus::DEVICE_ALREADY_INITIALIZED;
    }
    m_isPartiallyInitialized = true;

    try
    {
        m_cameraCore->initializeImager();
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "CameraDevice::initialize(): error initializing imager - " << e.what();
        return e.getStatus();
    }
    try
    {
        // cache imager serial
        m_imagerSerial = m_cameraCore->getImagerSerialNumber();
    }
    catch (Exception &e)
    {
        // reachable if the camera is unplugged before initialize() is called
        LOG (ERROR) << "CameraDevice::initialize(): error reading from camera - " << e.what();
        return e.getStatus();
    }

    m_cameraName = m_config->getCameraName();

    m_processing->setCameraName (m_cameraName);

    m_processing->registerExposureListener (this);

    m_exposureModes.clear();

    // make a (modifiable) copy of the usecases
    m_availableUseCases.clear();

    if (m_useExternalTrigger)
    {
        // If we use an external trigger raise the target rate
        // of the slave so that the sequence on the slave is in any case
        // done before receiving the next trigger signal from the master
        // We also have to adapt the exposure limits in this way :
        // new_max_exposure = old_max_exposure * old_framerate / new_framerate
        for (const auto &uc : m_config->getSupportedUseCases())
        {
            std::shared_ptr<UseCaseDefinition> slaveUCD = std::make_shared<UseCaseSlave> (*uc.getDefinition());
            UseCase newUseCase (uc.getName(), slaveUCD, uc.getProcessingParameters(),
                                uc.getCallbackData(), uc.getAccessLevel());

            m_availableUseCases.emplace_back (newUseCase);
        }
    }
    else
    {
        for (const auto &uc : m_config->getSupportedUseCases())
        {
            m_availableUseCases.emplace_back (uc);
        }
    }

    if (m_calibrationData.empty())
    {
        // if there was no previously assigned calibration data
        // get the calibration data from the camera module (if available)
        try
        {
            m_calibrationData = m_cameraCore->getCalibrationData();
        }
        catch (Exception &e)
        {
            LOG (WARN) << "CameraDevice::initialize(): module has no valid calibration information - " << e.what();

            if (m_callbackData != static_cast<uint16_t> (CallbackData::Raw))
            {
                return CameraStatus::NO_CALIBRATION_DATA;
            }
        }
    }

    if (!m_calibrationData.empty())
    {
        try
        {
            m_processing->setCalibrationData (m_calibrationData.toStdVector());
        }
        catch (Exception &e)
        {
            LOG (WARN) << "CameraDevice::initialize(): invalid assigned calibration data - " << e.what();
            if (m_callbackData != static_cast<uint16_t> (CallbackData::Raw))
            {
                return CameraStatus::CALIBRATION_DATA_ERROR;
            }
        }
    }

    if (m_callbackData != static_cast<uint16_t> (CallbackData::Raw) &&
            m_processing->needsCalibrationData() &&
            m_calibrationData.empty())
    {
        // We need calibration data to provide calculated values
        return CameraStatus::CALIBRATION_DATA_ERROR;
    }

    updateSupportedUseCases();

    if (m_supportedUseCaseNames.empty() &&
            !m_availableUseCases.empty ())
    {
        LOG (ERROR) << "There are no use cases available for the current access level";
        return CameraStatus::NO_USE_CASES_FOR_LEVEL;
    }

    if (m_availableUseCases.empty())
    {
        LOG (ERROR) << "There are no use cases available";
        return CameraStatus::NO_USE_CASES;
    }

    initLensOffset();

    m_isInitialized = true;

    auto ret = setUseCase (0);
    m_isInitialized = (ret == CameraStatus::SUCCESS);
    return ret;
}
ROYALE_API_EXCEPTION_SAFE_END

void CameraDevice::initLensOffset()
{
    m_config->getLensCenterDesign (m_lensCenter.first, m_lensCenter.second);
    try
    {
        if (m_processing->hasLensCenterCalibration())
        {
            uint16_t lensCalibX{ 0 }, lensCalibY{ 0 };
            m_processing->getLensCenterCalibration (lensCalibX, lensCalibY);

            auto relColOffset = narrow_cast<int16_t> (lensCalibX - m_lensCenter.first);
            auto relRowOffset = narrow_cast<int16_t> (lensCalibY - m_lensCenter.second);
            m_cameraCore->setLensOffset (relColOffset, relRowOffset);
            m_lensCenter.first = static_cast<uint16_t> (m_lensCenter.first + relColOffset);
            m_lensCenter.second = static_cast<uint16_t> (m_lensCenter.second + relRowOffset);
        }
    }
    catch (Exception &e)
    {
        LOG (WARN) << "CameraDevice::initialize(): error during setting lens offset - " << e.what();
    }
}

CameraStatus CameraDevice::startCapture() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    if (isCapturing())
    {
        return CameraStatus::SUCCESS;
    }

    {
        std::lock_guard<std::recursive_mutex> lock (m_exposureListenerMutex);
        if (m_exposureListener1 != nullptr || m_exposureListener2 != nullptr)
        {
            // The IExposureListener only receives one exposure time, which for non-mixed 9-frame
            // use cases was the exposure time of the first modulated phase.  For mixed mode, it's
            // the first modulated phase of the first stream that contains a modulated phase.
            const auto &rawSets = m_currentUseCaseDefinition->getRawFrameSets();
            for (const auto &streamId : m_currentUseCaseDefinition->getStreamIds())
            {
                // Only the first frame group is needed - if it doesn't have a modulated RFS then
                // the other frame groups in this stream don't have one either.
                for (const auto &rawSetIndex : m_currentUseCaseDefinition->getRawFrameSetIndices (streamId, 0))
                {
                    const auto &rawSet = rawSets[rawSetIndex];
                    if (rawSet.isModulated())
                    {
                        // Use exposure time from first modulated phase
                        auto exposure = m_currentUseCaseDefinition->getExposureTimeForRawFrameSet (rawSet);
                        if (m_exposureListener1)
                        {
                            m_exposureListener1->onNewExposure (exposure);
                        }
                        if (m_exposureListener2)
                        {
                            m_exposureListener2->onNewExposure (exposure, streamId);
                        }
                        break;
                    }
                }
            }
        }
    }

    try
    {
        m_cameraCore->startCapture();
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "CameraDevice::startCapture() : error starting capturing : " << e.what();
        return e.getStatus();
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::stopCapture() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!isCapturing())
    {
        return CameraStatus::SUCCESS;
    }

    try
    {
        m_cameraCore->stopCapture();
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "CameraDevice::stopCapture() : Error stopping capturing : " << e.what();
        return e.getStatus();
    }
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getMaxSensorWidth (uint16_t &maxSensorWidth) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    maxSensorWidth = m_config->getMaxImageWidth();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END


CameraStatus CameraDevice::getMaxSensorHeight (uint16_t &maxSensorHeight) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    maxSensorHeight = m_config->getMaxImageHeight();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setExposureTime (uint32_t exposureTime, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    normalizeStreamId (streamId);

    // Check if auto exposure is enabled
    try
    {
        if (isAutoexposureEnabled (streamId))
        {
            return CameraStatus::EXPOSURE_MODE_INVALID;
        }
    }
    catch (...)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    return updateExposureTime (exposureTime, streamId);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setExposureMode (ExposureMode exposureMode, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    normalizeStreamId (streamId);

    if (m_useExternalTrigger)
    {
        return CameraStatus::NOT_IMPLEMENTED;
    }

    ExposureMode currentExposureMode;
    auto ret = getExposureMode (currentExposureMode, streamId);
    if (ret != CameraStatus::SUCCESS)
    {
        return ret;
    }

    if (currentExposureMode == exposureMode)
    {
        return CameraStatus::SUCCESS;
    }

    if (exposureMode == ExposureMode::AUTOMATIC &&
            !m_config->isAutoExposureSupported())
    {
        return CameraStatus::NOT_IMPLEMENTED;
    }

    try
    {
        m_processing->setExposureMode (exposureMode, streamId);
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Error setting exposure mode : " << e.what();
        return CameraStatus::RUNTIME_ERROR;
    }

    m_exposureModes[streamId] = exposureMode;

    ProcessingParameterMap newparameters;
    m_processing->getProcessingParameters (newparameters, streamId);

    for (auto i = 0u; i < m_streams.size(); ++i)
    {
        if (m_streams[i] == streamId)
        {
            m_currentParameters[i] = newparameters;
        }
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getExposureMode (royale::ExposureMode &exposureMode, royale::StreamId streamId) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    normalizeStreamId (streamId);

    exposureMode = m_processing->getExposureMode (streamId);

    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getExposureLimits (Pair<uint32_t, uint32_t> &exposureLimits, royale::StreamId streamId) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }
    normalizeStreamId (streamId);
    exposureLimits = m_currentUseCaseDefinition->getExposureLimits (streamId);
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setFrameRate (uint16_t framerate) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);

    // \todo ROYAL-1783 Proper implementation for mixed mode (as soon as the semantics are clarified)
    if (m_currentUseCaseDefinition->getStreamIds().count() > 1)
    {
        return CameraStatus::NOT_IMPLEMENTED;
    }

    // ensure that the framerate is in the allowed min/max range
    if (framerate < m_currentUseCaseDefinition->getMinRate() ||
            framerate > m_currentUseCaseDefinition->getMaxRate())
    {
        return CameraStatus::FRAMERATE_NOT_SUPPORTED;
    }

    auto savedTargetRate = m_currentUseCaseDefinition->getTargetRate();
    m_currentUseCaseDefinition->setTargetRate (framerate);

    try
    {
        // do not update frame collector because buffer size will not change
        m_cameraCore->reconfigureImagerTargetFrameRate (framerate);
    }
    catch (common::Exception &e)
    {
        LOG (ERROR) << "CameraDevice::setFrameRate() : Framerate could not be set " << e.what();
        m_currentUseCaseDefinition->setTargetRate (savedTargetRate);
        return e.getStatus();
    }
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getFrameRate (uint16_t &frameRate) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    frameRate = m_currentUseCaseDefinition->getTargetRate();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getMaxFrameRate (uint16_t &maxFrameRate) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }
    maxFrameRate = m_currentUseCaseDefinition->getMaxRate();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END


CameraStatus CameraDevice::getStreams (royale::Vector<royale::StreamId> &streams) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    streams.clear();
    if (m_currentUseCaseDefinition)
    {
        streams = m_currentUseCaseDefinition->getStreamIds();
    }
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END


CameraStatus CameraDevice::registerEventListener (royale::IEventListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    auto status = CameraDeviceBase::registerEventListener (listener);
    if (status != CameraStatus::SUCCESS)
    {
        LOG (ERROR) << "CameraDeviceBase::s registerEventListener method failed inside Cameras registerEventListener method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
    }

    m_cameraCore->setEventListener (listener);
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::unregisterEventListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    CameraDeviceBase::unregisterEventListener();
    m_cameraCore->setEventListener (nullptr);
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

void CameraDevice::onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId)
{
    // Check if auto exposure is enabled
    try
    {
        if (!isAutoexposureEnabled (streamId))
        {
            // If autoexposure is not enabled this will not change the
            // exposure time
            return;
        }
    }
    catch (...)
    {
        return;
    }

    if (updateExposureTime (exposureTime, streamId) != CameraStatus::SUCCESS)
    {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock (m_exposureListenerMutex);
    if (m_exposureListener1 != nullptr)
    {
        m_exposureListener1->onNewExposure (exposureTime);
    }
    if (m_exposureListener2 != nullptr)
    {
        m_exposureListener2->onNewExposure (exposureTime, streamId);
    }
}

// Previously a deprecated public function, now a private one
bool CameraDevice::isCapturing() const
{
    return m_cameraCore->isCapturing();
}

CameraStatus CameraDevice::isConnected (bool &connected) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    connected = m_cameraCore->isConnected();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::isCapturing (bool &capturing) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    capturing = m_cameraCore->isCapturing();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::startRecording (const String &fileName,
        uint32_t numberOfFrames,
        uint32_t frameSkip,
        uint32_t msSkip) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (m_accessLevel > CameraAccessLevel::L3)
    {
        //it cannot be guaranteed that recording at L4 works
        return CameraStatus::NOT_IMPLEMENTED;
    }

    if (!m_recording)
    {
        return CameraStatus::RESOURCE_ERROR;
    }

    // redirect the capture listener to recording
    m_cameraCore->setCaptureListener (m_recording.get());

    try
    {
        m_recording->startRecord (fileName, m_calibrationData.toStdVector(),
                                  m_imagerSerial,
                                  numberOfFrames, frameSkip, msSkip);
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "CameraDevice::startRecording : cannot start recording, " << e.what();
        return CameraStatus::COULD_NOT_OPEN;
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::stopRecording() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (m_accessLevel > CameraAccessLevel::L3)
    {
        //it cannot be guaranteed that recording at L4 works
        return CameraStatus::NOT_IMPLEMENTED;
    }

    if (!m_recording)
    {
        return CameraStatus::RESOURCE_ERROR;
    }

    m_recording->stopRecord();

    auto status = setupListeners();
    if (status != CameraStatus::SUCCESS)
    {
        LOG (ERROR) << "Cameras setupListeners method failed inside its stopRecording method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

void CameraDevice::onRecordingStopped (const uint32_t numFrames)
{
    std::lock_guard<std::mutex> lck (m_recordingMutex);

    m_cameraCore->setCaptureListener (m_processing.get());

    if (m_recordStopListener != nullptr)
    {
        m_recordStopListener->onRecordingStopped (numFrames);
    }
}

CameraStatus CameraDevice::setDutyCycle (double dutyCycle, uint16_t index) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);

    if (index >= m_currentUseCaseDefinition->getRawFrameSets().size())
    {
        return CameraStatus::OUT_OF_BOUNDS;
    }

    try
    {
        m_currentUseCaseDefinition->setDutyCycle (dutyCycle, &m_currentUseCaseDefinition->getRawFrameSets().at (index));
    }
    catch (OutOfBounds &e)
    {
        LOG (ERROR) << "CameraDevice::setDutyCycle() : DutyCycle could not be set " << e.what();
        return CameraStatus::DUTYCYCLE_NOT_SUPPORTED;
    }

    try
    {
        return activateUseCase();
    }
    catch (common::Exception &e)
    {
        LOG (ERROR) << "CameraDevice::setDutyCycle() : DutyCycle could not be set " << e.what();
        return e.getStatus();
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setExposureTimes (const Vector<uint32_t> &exposureTimes, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    normalizeStreamId (streamId);

    // Check if auto exposure is enabled
    try
    {
        if (isAutoexposureEnabled (streamId))
        {
            return CameraStatus::EXPOSURE_MODE_INVALID;
        }
    }
    catch (...)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    auto &exposureGroups = m_currentUseCaseDefinition->getExposureGroups();
    const auto rawFrameSets = m_currentUseCaseDefinition->getRawFrameSets();
    auto frameSetsIdxs = m_currentUseCaseDefinition->getRawFrameSetIndices (streamId, 0);

    if (exposureTimes.size() < frameSetsIdxs.size())
    {
        return CameraStatus::OUT_OF_BOUNDS;
    }

    Vector<uint32_t> newExposures;
    newExposures.resize (exposureGroups.size());
    for (auto i = 0u; i < frameSetsIdxs.size(); ++i)
    {
        const auto expoGroupIdx = rawFrameSets[frameSetsIdxs[i]].exposureGroupIdx;
        newExposures[expoGroupIdx] = m_processing->getRefinedExposureTime (exposureTimes[i], exposureGroups[expoGroupIdx].m_exposureLimits);

        // check if exposure is compliant with the selected operation mode
        if (m_accessLevel < CameraAccessLevel::L3 &&
                (newExposures[expoGroupIdx] < exposureGroups[expoGroupIdx].m_exposureLimits.first ||
                 newExposures[expoGroupIdx] > exposureGroups[expoGroupIdx].m_exposureLimits.second))
        {
            return CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED;
        }
    }

    return updateExposureTimes (newExposures);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setExposureForGroups (const Vector<uint32_t> &exposureTimes) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    auto streamIds = m_currentUseCaseDefinition->getStreamIds();
    for (auto streamId : streamIds)
    {
        // Check if auto exposure is enabled for any of the streams
        try
        {
            if (isAutoexposureEnabled (streamId))
            {
                return CameraStatus::EXPOSURE_MODE_INVALID;
            }
        }
        catch (...)
        {
            return CameraStatus::RUNTIME_ERROR;
        }
    }

    auto exposureGroups = m_currentUseCaseDefinition->getExposureGroups();
    if (exposureTimes.size() < exposureGroups.size())
    {
        return CameraStatus::OUT_OF_BOUNDS;
    }

    Vector<uint32_t> newExposures;
    newExposures.resize (exposureGroups.size());
    for (auto i = 0u; i < exposureGroups.size(); ++i)
    {
        newExposures[i] = m_processing->getRefinedExposureTime (exposureTimes[i], exposureGroups[i].m_exposureLimits);

        // check if exposure is compliant with the selected operation mode
        if (m_accessLevel < CameraAccessLevel::L3 &&
                (newExposures[i] < exposureGroups[i].m_exposureLimits.first || newExposures[i] > exposureGroups[i].m_exposureLimits.second))
        {
            return CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED;
        }
    }

    return updateExposureTimes (newExposures);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setExposureTime (const String &exposureGroup, uint32_t exposureTime) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    auto streamIds = m_currentUseCaseDefinition->getStreamIds();
    for (auto streamId : streamIds)
    {
        // Check if auto exposure is enabled
        try
        {
            if (isAutoexposureEnabled (streamId))
            {
                return CameraStatus::EXPOSURE_MODE_INVALID;
            }
        }
        catch (...)
        {
            return CameraStatus::RUNTIME_ERROR;
        }
    }

    auto exposureGroups = m_currentUseCaseDefinition->getExposureGroups();
    auto exposureTimes  = m_currentUseCaseDefinition->getExposureTimes();

    for (auto idx = 0u; idx < exposureGroups.size(); ++idx)
    {
        const auto &e = exposureGroups.at (idx);
        if (e.m_name == exposureGroup)
        {
            exposureTimes.at (idx) = m_processing->getRefinedExposureTime (exposureTime, e.m_exposureLimits);

            // check if exposure is compliant with the selected operation mode
            if (m_accessLevel < CameraAccessLevel::L3 && (exposureTimes.at (idx) < e.m_exposureLimits.first || exposureTimes.at (idx) > e.m_exposureLimits.second))
            {
                return CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED;
            }
            return updateExposureTimes (exposureTimes);
        }
    }
    return CameraStatus::INVALID_VALUE;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getExposureGroups (royale::Vector< royale::String > &exposureGroups) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    exposureGroups.clear();
    for (const auto &group : m_currentUseCaseDefinition->getExposureGroups())
    {
        exposureGroups.emplace_back (group.m_name);
    }
    return CameraStatus::SUCCESS;
}

ROYALE_API_EXCEPTION_SAFE_END
CameraStatus CameraDevice::getExposureLimits (const String &exposureGroup, royale::Pair<uint32_t, uint32_t> &exposureLimits) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    auto exposureGroups = m_currentUseCaseDefinition->getExposureGroups();

    for (auto idx = 0u; idx < exposureGroups.size(); ++idx)
    {
        const auto &e = exposureGroups.at (idx);
        if (e.m_name == exposureGroup)
        {
            exposureLimits = e.m_exposureLimits;
            return CameraStatus::SUCCESS;
        }
    }
    return CameraStatus::INVALID_VALUE;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::getCameraInfo (Vector<Pair<String, String>> &camInfo) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    camInfo = m_cameraCore->getCoreInfo();

    auto procInfo = m_processing->getProcessingInfo();

    for (const auto &curInfo : procInfo)
    {
        camInfo.emplace_back (curInfo);
    }

    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setUseCase (size_t idx) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    if (idx >= m_availableUseCases.size())
    {
        return CameraStatus::INVALID_VALUE;
    }

    std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);

    if (m_currentUseCaseDefinition == m_availableUseCases[idx].getDefinition())
    {
        return CameraStatus::SUCCESS;
    }

    if (m_accessLevel < m_availableUseCases[idx].getAccessLevel())
    {
        return CameraStatus::INSUFFICIENT_PRIVILEGES;
    }

    // Check Processing if required
    if (m_callbackData > static_cast<uint16_t> (CallbackData::Raw) &&
            m_processing->verifyUseCase (*m_availableUseCases[idx].getDefinition()) != royale::usecase::VerificationStatus::SUCCESS)
    {
        LOG (ERROR) << "CameraDevice::setUseCase() : Usecase mode is not supported by Processing";
        return CameraStatus::USECASE_NOT_SUPPORTED;
    }

    auto lastParameters = m_currentParameters;
    auto lastDefiniton = m_currentUseCaseDefinition;
    auto lastUseCaseName = m_currentUseCaseName;

    m_currentParameters = m_availableUseCases[idx].getProcessingParameters();
    m_currentUseCaseDefinition = m_availableUseCases[idx].getDefinition();
    m_currentUseCaseName = m_availableUseCases[idx].getName();

    auto activeModeEnabled = false;
    if (isCapturing())
    {
        activeModeEnabled = true;
    }

    auto status = activateUseCase();

    if (status != CameraStatus::SUCCESS)
    {
        // activating the use case didn't work, reactivate the last working one
        m_currentParameters = lastParameters;
        m_currentUseCaseDefinition = lastDefiniton;
        m_currentUseCaseName = lastUseCaseName;

        auto innerStatus = activateLastUseCase (activeModeEnabled);
        if (innerStatus != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras activateLastUseCase method failed inside its setUseCase method. CameraStatus = \"" << royale::getStatusString (innerStatus) << "\"";
        }

        return status;
    }
    else
    {
        return CameraStatus::SUCCESS;
    }
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::activateLastUseCase (bool wasActive)
{
    //return if last use case is invalid
    if (!m_currentUseCaseDefinition)
    {
        return CameraStatus::USECASE_NOT_SUPPORTED;
    }

    auto status = activateUseCase();
    if (status != CameraStatus::SUCCESS)
    {
        // we couldn't recover the last use case
        return CameraStatus::RUNTIME_ERROR;
    }

    // we were able to set the old use case
    if (wasActive &&
            !isCapturing())
    {
        // if we were capturing before, but are not capturing
        // anymore, try to start capturing again
        auto innerStatus = startCapture();
        if (innerStatus != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras startCapture method failed inside its activateLastUseCase method. CameraStatus = \"" << royale::getStatusString (innerStatus) << "\"";
        }
    }

    return CameraStatus::USECASE_NOT_SUPPORTED;
}

CameraStatus CameraDevice::setUseCase (const royale::String &name) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    for (size_t i = 0; i < m_availableUseCases.size(); ++i)
    {
        if (m_availableUseCases[i].getName() == name)
        {
            return setUseCase (i);
        }
    }
    LOG (ERROR) << "UseCase is not supported by the camera device";
    return CameraStatus::USECASE_NOT_SUPPORTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setCallbackData (uint16_t cbData) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    if (cbData != static_cast<uint16_t> (CallbackData::Raw) &&
            cbData != static_cast<uint16_t> (CallbackData::Depth) &&
            cbData != static_cast<uint16_t> (CallbackData::Intermediate))
    {
        return CameraStatus::INVALID_VALUE;
    }

    if (cbData >  static_cast<uint16_t> (CallbackData::Raw) &&
            (m_isInitialized && !m_processing->isReadyToProcessDepthData()))
    {
        return CameraStatus::NO_CALIBRATION_DATA;
    }

    bool restartCapture { false };
    if (isCapturing())
    {
        restartCapture = true;
        auto status = stopCapture();
        if (status != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras stopCapture method failed inside its setCallbackData method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
        }
    }

    // If we switch from raw to depth we have to make sure that
    // the processing is set up correctly and that the use case
    // is also available for depth
    bool switchFromRaw = false;
    if (m_callbackData == static_cast<uint16_t> (CallbackData::Raw) &&
            cbData >  static_cast<uint16_t> (CallbackData::Raw))
    {
        switchFromRaw = true;
    }

    uint16_t oldCbData = m_callbackData;

    m_callbackData = cbData;
    updateSupportedUseCases();

    CameraStatus switchStatus = CameraStatus::SUCCESS;

    if (m_isInitialized && switchFromRaw)
    {
        std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);

        // We try to switch from raw to one of the depth modes
        m_currentUseCaseDefinition = nullptr;

        // Find out if the previous use case is still valid
        bool useCaseStillValid = false;

        for (const auto curUseCase : m_supportedUseCaseNames)
        {
            if (m_currentUseCaseName == curUseCase)
            {
                useCaseStillValid = true;
                break;
            }
        }

        if (useCaseStillValid)
        {
            auto status = setUseCase (m_currentUseCaseName);
            if (status != CameraStatus::SUCCESS)
            {
                LOG (ERROR) << "Cameras setUseCase method failed inside its setCallbackData method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
            }
        }
        else
        {
            if (!m_supportedUseCaseNames.empty())
            {
                auto status = setUseCase (m_supportedUseCaseNames.at (0));
                if (status != CameraStatus::SUCCESS)
                {
                    LOG (ERROR) << "Cameras setUseCase method failed inside its setCallbackData method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
                }
            }
            else
            {
                // We have no supported use cases we could switch to
                m_callbackData = oldCbData;
                updateSupportedUseCases();
                switchStatus = CameraStatus::RUNTIME_ERROR;

                auto status = setUseCase (m_currentUseCaseName);
                if (status != CameraStatus::SUCCESS)
                {
                    LOG (ERROR) << "Cameras setUseCase method failed inside its setCallbackData method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
                }
            }
        }

        // We have to initialize the processing again in case we
        // didn't set the calibration data yet or the calibration
        // data changed
        try
        {
            m_processing->setCalibrationData (m_calibrationData.toStdVector());
        }
        catch (Exception &e)
        {
            LOG (WARN) << "CameraDevice::initialize(): invalid assigned calibration data - " << e.what();
            return CameraStatus::CALIBRATION_DATA_ERROR;
        }

        CameraStatus ret = CameraStatus::RUNTIME_ERROR;
        try
        {
            ret = activateUseCase();
        }
        catch (const Exception &)
        {
            return ret;
        }
    }

    auto status = setupListeners();
    if (status != CameraStatus::SUCCESS)
    {
        LOG (ERROR) << "Cameras setupListeners method failed inside its setCallbackData method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
    }

    if (restartCapture)
    {
        status = startCapture();
        if (status != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras startCapture method failed inside its setCallbackData method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
        }
    }

    // update recording engine
    CameraStatus recordingStatus = updateRecordingEngineListener();

    if (switchStatus != CameraStatus::SUCCESS)
    {
        return switchStatus;
    }
    else
    {
        return recordingStatus;
    }
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::updateRecordingEngineListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (m_recording)
    {
        if (!m_recording->setFrameCaptureListener (m_processing.get()))
        {
            return CameraStatus::DEVICE_IS_BUSY;
        }
    }
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

void CameraDevice::scaleExposuresForUseCase()
{
    auto exposureTimes = m_currentUseCaseDefinition->getExposureTimes();
    auto expoGroups = m_currentUseCaseDefinition->getExposureGroups();

    for (auto idx = 0u; idx < exposureTimes.size(); ++idx)
    {
        exposureTimes[idx] = m_processing->getRefinedExposureTime (exposureTimes[idx], expoGroups[idx].m_exposureLimits);
    }
    m_currentUseCaseDefinition->setExposureTimes (royale::Vector<uint32_t>::toStdVector (exposureTimes));
}

CameraStatus CameraDevice::activateUseCase() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    // bring camera to ready state before setting up new use case
    auto activeModeEnabled = false;
    if (isCapturing())
    {
        activeModeEnabled = true;

        auto status = stopCapture();
        if (status != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras stopCapture method failed inside its activateUseCase method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
        }
    }

    // Adapt exposure times based on the scaling factor
    scaleExposuresForUseCase();

    // Activate CameraModule
    try
    {
        m_cameraCore->setUseCase (m_currentUseCaseDefinition);
    }
    catch (const ResourceError &)
    {
        return CameraStatus::INSUFFICIENT_BANDWIDTH;
    }
    catch (...)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    m_modulationFrequencies.clear();
    auto rawFrameSets = m_currentUseCaseDefinition->getRawFrameSets();
    auto streamIds = m_currentUseCaseDefinition->getStreamIds();
    const size_t numStreams = streamIds.size();

    m_streams.clear();
    for (auto i = 0u; i < numStreams; ++i)
    {
        m_streams.push_back (streamIds[i]);
        if (m_exposureModes.find (streamIds[i]) == m_exposureModes.end())
        {
            m_exposureModes[streamIds[i]] = ExposureMode::MANUAL;
        }

        if (m_currentUseCaseDefinition->getFrameGroupCount (streamIds[i]) == 0u)
        {
            continue;
        }

        const auto &frameSetIdxs = m_currentUseCaseDefinition->getRawFrameSetIndices (streamIds[i], 0);
        for (const auto frameSetIdx : frameSetIdxs)
        {
            const auto &rawFrameSet = rawFrameSets.at (frameSetIdx);
            m_modulationFrequencies[streamIds[i]].push_back (rawFrameSet.modulationFrequency);
        }

        auto limits = m_currentUseCaseDefinition->getExposureLimits (streamIds[i]);
        m_currentParameters[i][ProcessingFlag::AutoExpoMin_Int] = royale::Variant (static_cast<int> (limits.first),
                static_cast<int> (limits.first), static_cast<int> (limits.second));
        m_currentParameters[i][ProcessingFlag::AutoExpoMax_Int] = royale::Variant (static_cast<int> (limits.second),
                static_cast<int> (limits.first), static_cast<int> (limits.second));
    }


    if (m_recording)
    {
        m_recording->resetParameters();
        for (auto i = 0u; i < numStreams; ++i)
        {
            m_recording->setProcessingParameters (ProcessingParameterVector::fromStdMap (m_currentParameters[i]), streamIds[i]);
        }
    }

    try
    {
        m_processing->setUseCase (*m_currentUseCaseDefinition);

        if (numStreams != m_currentParameters.size())
        {
            return CameraStatus::OUT_OF_BOUNDS;
        }

        for (auto i = 0u; i < numStreams; ++i)
        {
            try
            {
                m_processing->setProcessingParameters (m_currentParameters[i], streamIds[i]);

                auto autoexp = m_currentParameters[i].find (ProcessingFlag::UseAutoExposure_Bool);
                if (autoexp != m_currentParameters[i].end() && autoexp->second.getBool() == true)
                {
                    m_exposureModes[streamIds[i]] = ExposureMode::AUTOMATIC;
                }
            }
            catch (Exception &e)
            {
                if (m_callbackData != static_cast<uint16_t> (CallbackData::Raw))
                {
                    LOG (ERROR) << "Error setting processing parameters : " << e.what();
                    return CameraStatus::RUNTIME_ERROR;
                }
            }
        }
    }
    catch (const RuntimeError &e)
    {
        if (m_callbackData > static_cast<uint16_t> (CallbackData::Raw))
        {
            return CameraStatus::USECASE_NOT_SUPPORTED;
        }
        else
        {
            LOG (ERROR) << "Calibration data error : " << e.what();
        }
    }
    catch (const LogicError &)
    {
        return CameraStatus::SPECTRE_NOT_INITIALIZED;
    }
    catch (...)
    {
        return CameraStatus::UNKNOWN;
    }

    for (const auto &streamId : streamIds)
    {
        setExposureMode (m_exposureModes[streamId], streamId);
    }

    // if the camera was capturing before, bring again into capture mode
    if (activeModeEnabled)
    {
        auto status = startCapture();
        if (status != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "Cameras startCapture method failed inside its activateUseCase method. CameraStatus = \"" << royale::getStatusString (status) << "\"";
        }
    }

    // tell the listeners about the current exposure times
    callExposureListeners (*m_currentUseCaseDefinition);

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::writeCalibrationToFlash() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    try
    {
        m_cameraCore->flashCalibrationData (m_calibrationData.toStdVector());
    }
    catch (const LogicError &)
    {
        return CameraStatus::RESOURCE_ERROR;
    }
    catch (...)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::writeDataToFlash (const royale::Vector<uint8_t> &data) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    try
    {
        m_cameraCore->flashData (data.toStdVector());
    }
    catch (const LogicError &)
    {
        return CameraStatus::RESOURCE_ERROR;
    }
    catch (...)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::writeDataToFlash (const royale::String &filename) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    if (!fileexists (filename))
    {
        return CameraStatus::FILE_NOT_FOUND;
    }

    Vector<uint8_t> data;

    if (!readFileToVector (filename, data))
    {
        return CameraStatus::DATA_NOT_FOUND;
    }

    return writeDataToFlash (data);
} ROYALE_API_EXCEPTION_SAFE_END

void CameraDevice::saveProcessingParameters (royale::StreamId streamId)
{
    if (m_recording)
    {
        // Retrieve the full parameter set from the processing
        // setProcessingParameters might only receive a partial list of parameters
        ProcessingParameterMap newparameters;
        m_processing->getProcessingParameters (newparameters, streamId);

        m_recording->setProcessingParameters (ProcessingParameterVector::fromStdMap (newparameters), streamId);
    }
}

CameraStatus CameraDevice::setupListeners() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (m_callbackData > static_cast<uint16_t> (CallbackData::Raw) &&
            (m_isInitialized && !m_processing->isReadyToProcessDepthData()))
    {
        return CameraStatus::NO_CALIBRATION_DATA;
    }

    if (m_callbackData == static_cast<uint16_t> (CallbackData::Depth))
    {
        LOG (DEBUG) << "Setup listeners for DEPTH data";

        m_processing->registerDataListeners (m_listeners);
    }
    else if (m_callbackData == static_cast<uint16_t> (CallbackData::Raw))
    {
        LOG (DEBUG) << "Setup listeners for RAW data";

        m_processing->registerDataListeners (m_listeners);
    }
    else if (m_callbackData == static_cast<uint16_t> (CallbackData::Intermediate))
    {
        LOG (DEBUG) << "Setup listeners for INTERMEDIATE data";

        if (m_listeners.extendedListener)
        {
            m_processing->registerDataListeners (m_listeners);
        }
        else
        {
            return CameraStatus::RUNTIME_ERROR;
        }
    }
    else
    {
        LOG (ERROR) << "Specified callback data " << m_callbackData << " is not supported.";
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::initialize (const String &initUseCase) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L4)

    m_isInitialized = false;

    // First do a normal initialization
    auto ret = initialize();

    if (ret != CameraStatus::SUCCESS)
    {
        return ret;
    }

    //setUseCase needs the device to be initialized
    m_isInitialized = true;

    // If the initialization succeeds, set the desired use case
    ret = setUseCase (initUseCase);

    if (ret != CameraStatus::SUCCESS)
    {
        m_isInitialized = false;
    }

    return ret;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::writeRegisters (
    const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    try
    {
        m_cameraCore->writeRegisters (registers);
    }
    catch (...)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::readRegisters (
    royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    try
    {
        m_cameraCore->readRegisters (registers);
    }
    catch (...)
    {
        return CameraStatus::RUNTIME_ERROR;
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

royale::CameraStatus CameraDevice::shiftLensCenter (int16_t tx, int16_t ty) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    if (m_callbackData != static_cast<uint16_t> (CallbackData::Raw))
    {
        LOG (ERROR) << "shiftLensCenter can only be used in Raw mode";
        return CameraStatus::LOGIC_ERROR;
    }

    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    CameraStatus ret;

    // Stop the capturing while trying the new lens center
    auto activeModeEnabled = false;
    if (isCapturing())
    {
        activeModeEnabled = true;
        ret = stopCapture();
        if (ret != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "CameraDevice::shiftLensCenter : unable to stop capturing";
            return ret;
        }
    }

    uint16_t centerCol, centerRow;
    m_config->getLensCenterDesign (centerCol, centerRow);
    int16_t relX = static_cast<int16_t> (m_lensCenter.first - centerCol);
    int16_t relY = static_cast<int16_t> (m_lensCenter.second - centerRow);

    m_cameraCore->setLensOffset (static_cast<int16_t> (relX + tx), static_cast<int16_t> (relY + ty));
    auto verificationStatus = m_cameraCore->verifyUseCase (m_currentUseCaseDefinition);

    bool newCenterWorks = true;

    if (verificationStatus != royale::usecase::VerificationStatus::SUCCESS)
    {
        // Take back lens center shift
        m_cameraCore->setLensOffset (relX, relY);
        LOG (ERROR) << "CameraDevice::shiftLensCenter(): error verifying lens offset";
        newCenterWorks = false;
    }

    if (newCenterWorks)
    {
        try
        {
            ret = activateUseCase();
        }
        catch (Exception &e)
        {
            // Take back lens center shift
            m_cameraCore->setLensOffset (relX, relY);
            ret = activateUseCase();
            LOG (ERROR) << "CameraDevice::shiftLensCenter(): error setting lens offset - " << e.what();
            newCenterWorks = false;
            if (ret != CameraStatus::SUCCESS)
            {
                LOG (ERROR) << "CameraDevice::shiftLensCenter : unable to activate old use case";
                return ret;
            }
        }

        if (ret != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "CameraDevice::shiftLensCenter : unable to activate use case";
            return ret;
        }

        if (newCenterWorks)
        {
            m_lensCenter.first = static_cast<uint16_t> (m_lensCenter.first + tx);
            m_lensCenter.second = static_cast<uint16_t> (m_lensCenter.second + ty);
        }
    }

    // if the camera was capturing before, bring again into capture mode
    if (activeModeEnabled)
    {
        ret = startCapture();
        if (ret != CameraStatus::SUCCESS)
        {
            LOG (ERROR) << "CameraDevice::shiftLensCenter : unable to restart capturing";
            return ret;
        }
    }

    if (!newCenterWorks)
    {
        return CameraStatus::INVALID_VALUE;
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

royale::CameraStatus CameraDevice::getLensCenter (uint16_t &x, uint16_t &y) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L3)

    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    x = m_lensCenter.first;
    y = m_lensCenter.second;

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

bool CameraDevice::isAutoexposureEnabled (const royale::StreamId streamId)
{
    return m_exposureModes[streamId] == ExposureMode::AUTOMATIC;
}

royale::CameraStatus CameraDevice::updateExposureTime (uint32_t exposureTime, royale::StreamId streamId)
{
    std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);

    const auto savedExposureTimes = m_currentUseCaseDefinition->getExposureTimes();

    // Check if exposure is compliant with the selected operation mode and limits
    const auto limits = m_currentUseCaseDefinition->getExposureLimits (streamId);
    const auto newExposure = m_processing->getRefinedExposureTime (exposureTime, limits);

    if (m_accessLevel < CameraAccessLevel::L3 &&
            (newExposure < limits.first || newExposure > limits.second))
    {
        return CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED;
    }

    try
    {
        // update use case and imager config
        m_currentUseCaseDefinition->setExposureTime (newExposure, streamId);
        if (isCapturing())
        {
            if (m_useExternalTrigger)
            {
                // If we're using an external trigger the reconfig counter will
                // never be incremented, because it is reset for a new trigger.
                // That's why we have to use a different way to reconfigure the
                // exposure
                m_cameraCore->stopCapture();
                m_cameraCore->setUseCase (m_currentUseCaseDefinition);
                m_cameraCore->startCapture();
            }
            else
            {
                m_cameraCore->reconfigureImagerExposureTimes (m_currentUseCaseDefinition, m_currentUseCaseDefinition->getExposureTimes().toStdVector());
            }
        }
        else
        {
            if (activateUseCase() != CameraStatus::SUCCESS)
            {
                throw RuntimeError ("Error activating the use case");
            }
        }
    }
    catch (const Exception &e)
    {
        m_currentUseCaseDefinition->setExposureTimes (savedExposureTimes);
        LOG (ERROR) << "CameraDevice::setExposureTime() : Error setting exposure time : " << e.what();
        return e.getStatus();
    }

    return CameraStatus::SUCCESS;
}

CameraStatus CameraDevice::getNumberOfStreams (const royale::String &name, uint32_t &nrStreams) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    for (size_t i = 0; i < m_availableUseCases.size(); ++i)
    {
        if (m_availableUseCases[i].getName() == name)
        {
            nrStreams = static_cast<uint32_t> (m_availableUseCases[i].getDefinition()->getStreamIds().size());
            return CameraStatus::SUCCESS;
        }
    }

    return CameraStatus::INVALID_VALUE;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDevice::setExternalTrigger (bool useExternalTrigger) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (m_isInitialized)
    {
        // this function should only be called on uninitialized CameraDevices
        return CameraStatus::DEVICE_ALREADY_INITIALIZED;
    }

    try
    {
        m_cameraCore->setExternalTrigger (useExternalTrigger);
    }
    catch (const WrongState &)
    {
        // It seems the imager was in the wrong state
        return CameraStatus::RUNTIME_ERROR;
    }
    catch (const InvalidValue &)
    {
        // The camera doesn't support external triggering
        // (at least that's what the ModuleConfig says)
        return CameraStatus::LOGIC_ERROR;
    }

    m_useExternalTrigger = useExternalTrigger;

    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END
