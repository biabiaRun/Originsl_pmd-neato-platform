/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <record/CameraPlayback.hpp>

#include <common/exceptions/APIExceptionHandling.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/MakeUnique.hpp>

#include <collector/CapturedUseCase.hpp>
#include <record/UseCaseRecord.hpp>

#ifdef USE_SPECTRE
#include <processing/ProcessingSpectre.hpp>
#endif

#include <processing/ProcessingSimple.hpp>

#include <RoyaleLogger.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <FileSystem.hpp>
#include <factory/ImagerFactory.hpp>
#include <common/events/EventCaptureStream.hpp>

using namespace royale;
using namespace royale::record;
using namespace royale::processing;
using namespace royale::common;
using namespace royale::collector;
using namespace royale::processing;

CameraPlayback::CameraPlayback (CameraAccessLevel level, const std::string &filename) :
    CameraDeviceBase (level, "", "UNINITIALIZED CAMERA", nullptr),
    m_captureListener (nullptr),
    m_isCapturing (false),
    m_filename (filename),
    m_timestampsUsed (true),
    m_loop (true),
    m_oneShot (false),
    m_stopListener (nullptr),
    m_seeked (false),
    m_eventForwarder()
{
    m_supportedUseCaseNames.push_back ("MODE_PLAYBACK");

#ifdef USE_SPECTRE
    m_processing.reset (new ProcessingSpectre (this));
#else
    m_processing.reset (new ProcessingSimple (this));
#endif

    m_captureListener = m_processing.get();
}

CameraPlayback::~CameraPlayback()
{
    if (m_isCapturing)
    {
        stopCapture();
    }

    if (aquisitionThread.joinable())
    {
        aquisitionThread.join();
    }
}

CameraStatus CameraPlayback::initialize()
{
    // If device is already initialized, skip second initialization to avoid unknown behavior.
    // For playback this isn't as complex as a CameraDevice, but reinitializing while the
    // acquisition thread is running is likely to cause undefined behaviour.
    if (m_isPartiallyInitialized)
    {
        return CameraStatus::DEVICE_ALREADY_INITIALIZED;
    }
    m_isPartiallyInitialized = true;

    try
    {
        m_reader.open (m_filename.toStdString());
    }
    catch (const std::invalid_argument &)
    {
        return CameraStatus::FILE_NOT_FOUND;
    }
    catch (const std::length_error &)
    {
        return CameraStatus::DATA_NOT_FOUND;
    }
    catch (const std::runtime_error &)
    {
        return CameraStatus::RUNTIME_ERROR;
    }
    catch (const std::logic_error &)
    {
        return CameraStatus::LOGIC_ERROR;
    }
    catch (...)
    {
        return CameraStatus::UNKNOWN;
    }

    m_id = m_reader.imagerSerial();

    m_pseudoDataInterpreter = factory::ImagerFactory::createPseudoDataInterpreter (m_reader.pseudoDataInterpreterType());

    m_cameraName = m_reader.cameraName();

    m_currentUseCaseName = "MODE_PLAYBACK";

    m_processing->setCameraName (m_cameraName);

    if (m_calibrationData.empty() &&
            m_reader.hasCalibrationData())
    {
        m_calibrationData = Vector<uint8_t>::fromStdVector (m_reader.getCalibrationData());
    }

    // set previously assigned calibration data (if any)
    if (m_calibrationData.size() > 0)
    {
        try
        {
            m_processing->setCalibrationData (m_calibrationData.toStdVector());
        }
        catch (Exception &e)
        {
            LOG (WARN) << "CameraPlayback::initialize(): invalid assigned calibration data - " << e.what();
            if (m_callbackData != (uint16_t) CallbackData::Raw)
            {
                LOG (ERROR) << "Calibration data invalid";
                return CameraStatus::CALIBRATION_DATA_ERROR;
            }
        }
    }
    else if (m_calibrationData.empty() &&
             m_callbackData != (uint16_t) CallbackData::Raw)
    {
        return CameraStatus::CALIBRATION_DATA_ERROR;
    }

    m_isInitialized = true;

    return seek (0);
}

CameraStatus CameraPlayback::setUseCase (const String &name)
{
    if (name == "MODE_PLAYBACK")
    {
        return CameraStatus::SUCCESS;
    }
    return CameraStatus::USECASE_NOT_SUPPORTED;
}

CameraStatus CameraPlayback::startCapture()
{
    if (!m_isInitialized)
    {
        return CameraStatus::DEVICE_NOT_INITIALIZED;
    }

    if (m_isCapturing)
    {
        stopCapture();
    }

    setupListeners();

    resume();

    return CameraStatus::SUCCESS;
}

CameraStatus CameraPlayback::stopCapture()
{

    if (m_processing)
    {
        royale::processing::DataListeners emptyListeners;
        m_processing->registerDataListeners (emptyListeners);
    }
    pause();

    if (m_stopListener)
    {
        m_stopListener->onPlaybackStopped();
    }

    return CameraStatus::SUCCESS;
}

CameraStatus CameraPlayback::getMaxSensorWidth (uint16_t &maxSensorWidth) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    maxSensorWidth = m_reader.getMaxWidth();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getMaxSensorHeight (uint16_t &maxSensorHeight) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    maxSensorHeight = m_reader.getMaxHeight();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::isConnected (bool &connected) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    connected = m_reader.isOpen();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::isCapturing (bool &capturing) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    capturing = m_isCapturing;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setCallbackData (uint16_t cbData)
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    m_callbackData = cbData;

    {
        // If there was a different callback data at the beginning
        // we might have missed the use case changes
        std::lock_guard<std::mutex> lck (m_playbackMutex);
        m_seeked = true;
    }

    setupListeners();

    return CameraStatus::SUCCESS;
}

void CameraPlayback::releaseCapturedFrames (std::vector<ICapturedRawFrame *> frame)
{

}

CameraStatus CameraPlayback::seek (const uint32_t frameNumber)
{
    {
        std::lock_guard<std::mutex> lck (m_playbackMutex);
        try
        {
            m_reader.seek (frameNumber);
        }
        catch (const std::logic_error &)
        {
            return CameraStatus::DATA_NOT_FOUND;
        }
        catch (...)
        {
            return CameraStatus::RUNTIME_ERROR;
        }

        m_seeked = true;

        if (!m_isCapturing)
        {
            m_oneShot = true;
        }
    }

    if (m_oneShot)
    {
        try
        {
            aquisitionFunction();
        }
        catch (Exception &e)
        {
            LOG (WARN) << "CameraPlayback::seek failed - " << e.what ();
            return CameraStatus::RUNTIME_ERROR;
        }
    }

    return CameraStatus::SUCCESS;
}

void CameraPlayback::loop (const bool restart)
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    m_loop = restart;
}

void CameraPlayback::useTimestamps (const bool timestampsUsed)
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    m_timestampsUsed = timestampsUsed;
}

uint32_t CameraPlayback::frameCount()
{
    return m_reader.numFrames();
}

uint32_t CameraPlayback::currentFrame()
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    return m_reader.currentFrame();
}

void CameraPlayback::pause()
{
    {
        std::lock_guard<std::mutex> lck (m_playbackMutex);
        if (!m_isCapturing)
        {
            return;
        }

        m_isCapturing = false;
    }

    if (std::this_thread::get_id() != aquisitionThread.get_id())
    {
        aquisitionThread.join();
    }
    else
    {
        aquisitionThread.detach();
    }

    // Take back the last increment from the thread
    uint32_t curFrame = m_reader.currentFrame();
    if (curFrame)
    {
        curFrame--;
        m_reader.seek (curFrame);
    }

}

void CameraPlayback::resume()
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    if (!m_isCapturing)
    {
        m_seeked = true;
        m_isCapturing = true;
        aquisitionThread = std::thread (&CameraPlayback::aquisitionFunction, this);
    }
}

void CameraPlayback::registerStopListener (IPlaybackStopListener *listener)
{
    m_stopListener = listener;
}

void CameraPlayback::unregisterStopListener()
{
    m_stopListener = nullptr;
}

namespace
{
    // \todo needs the v3 recording format
}

void CameraPlayback::aquisitionFunction()
{
    bool firstFrame = true;
    std::vector<std::vector<uint16_t>> frames;
    std::unique_ptr<royale::usecase::UseCaseDefinition> definition;
    std::chrono::milliseconds currentTimestampRecording;
    std::chrono::milliseconds lastTimestampRecording;
    std::chrono::milliseconds currentTimestampPlayback;
    std::chrono::milliseconds lastTimestampPlayback;
    float illuminationTemperature;
    ProcessingParameterMap parameterMap;
    uint32_t numFrames = m_reader.numFrames();
    std::vector<std::pair<std::string, std::vector<uint8_t>>> additionalData;
    StreamId streamId = 0;
    std::map<StreamId, bool> parametersSetForStream;
    while (true)
    {
        bool loop = true;
        bool timestampsUsed = true;
        {
            // Check if thread should still run
            std::lock_guard<std::mutex> lck (m_playbackMutex);
            if (!m_isCapturing && !m_oneShot)
            {
                return;
            }

            if (m_seeked)
            {
                firstFrame = true;
                m_seeked = false;
            }

            loop = m_loop;
            timestampsUsed = m_timestampsUsed;
        }

        std::unique_ptr<royale::usecase::UseCaseDefinition> oldDefinition = std::move (definition);
        readFrame (frames, definition, currentTimestampRecording, illuminationTemperature, m_capturedExposureTimes,
                   parameterMap, additionalData, streamId);

        if (!oldDefinition || *oldDefinition != *definition || firstFrame)
        {
            {
                std::lock_guard<std::recursive_mutex> lckIds (m_currentUseCaseMutex);
                m_streamIds = definition->getStreamIds();
            }

            m_modulationFrequencies.clear();
            auto rawFrameSets = definition->getRawFrameSets();
            for (auto rawFrameSet = rawFrameSets.begin(); rawFrameSet != rawFrameSets.end(); ++rawFrameSet)
            {
                m_modulationFrequencies.push_back (rawFrameSet->modulationFrequency);
            }

            parametersSetForStream.clear();
            for (auto curId : m_streamIds)
            {
                parametersSetForStream[curId] = false;
            }

            if (m_callbackData != (uint16_t) CallbackData::Raw)
            {
                std::lock_guard<std::mutex> lck (m_playbackMutex);
#ifdef USE_SPECTRE
                if (m_processing->hasCalibrationData())
#endif
                {
                    m_processing->setUseCase (*definition);
                }
            }

            // tell the listeners about the current exposure times
            callExposureListeners (*definition);
        }

        if (m_callbackData != (uint16_t) CallbackData::Raw &&
                parametersSetForStream[streamId] == false)
        {
            std::unique_lock<std::mutex> lck (m_playbackMutex);
            const auto &userParameters = m_userParameters.find (streamId);
            if (userParameters == m_userParameters.end() || userParameters->second.empty())
            {
                try
                {
                    m_processing->setProcessingParameters (parameterMap, streamId);
                }
                catch (Exception &e)
                {
                    lck.unlock();
                    LOG (ERROR) << "Error setting processing parameters : " << e.what();
                    m_eventForwarder.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_ERROR, "Error setting processing parameters");
                    stopCapture();
                    return;
                }
            }
            else
            {
                // Merge user parameters with the recorded ones
                for (auto curUserParameter : userParameters->second)
                {
                    parameterMap[curUserParameter.first] = curUserParameter.second;
                }
                try
                {
                    m_processing->setProcessingParameters (userParameters->second, streamId);
                }
                catch (Exception &e)
                {
                    lck.unlock();
                    LOG (ERROR) << "Error setting processing parameters : " << e.what();
                    m_eventForwarder.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_ERROR, "Error setting processing parameters");
                    stopCapture();
                    return;
                }
            }

            parametersSetForStream[streamId] = true;
        }

        if (firstFrame)
        {
            firstFrame = false;
            lastTimestampPlayback = std::chrono::duration_cast<std::chrono::milliseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
            lastTimestampRecording = currentTimestampRecording;
        }

        if (timestampsUsed)
        {
            currentTimestampPlayback = std::chrono::duration_cast<std::chrono::milliseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
            std::chrono::milliseconds durationPlayback = currentTimestampPlayback - lastTimestampPlayback;
            std::chrono::milliseconds durationRecording = currentTimestampRecording - lastTimestampRecording;

            std::chrono::milliseconds timeToSleep = durationRecording - durationPlayback;

            if (timeToSleep.count () > 0)
            {
                std::this_thread::sleep_for (timeToSleep);
            }
        }

        uint16_t columns, rows;
        definition->getImage (columns, rows);

        if (columns >= m_pseudoDataInterpreter->getRequiredImageWidth())
        {
            // we need to convert to microseconds for the upstream pipeline
            std::unique_ptr<CapturedUseCase> cuc{ new CapturedUseCase{ m_pseudoDataInterpreter.get(), illuminationTemperature,
                        std::chrono::duration_cast<std::chrono::microseconds> (currentTimestampRecording), m_capturedExposureTimes } };

            std::vector<ICapturedRawFrame *> recordedFrames;
            for (auto currentRawData = frames.begin(); currentRawData != frames.end(); ++currentRawData)
            {
                RecordedRawFrame *curFrame = new RecordedRawFrame (*currentRawData, columns);
                recordedFrames.push_back (curFrame);
            }

            internalCallback (recordedFrames, *definition, std::move (cuc), streamId);

            for (size_t idx = 0; idx < recordedFrames.size(); ++idx)
            {
                delete recordedFrames.at (idx);
            }

        }
        else
        {
            LOG (ERROR) << "Frame too small, stopping...";
            m_eventForwarder.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_ERROR, "Frame too small, recording is possibly broken");
            stopCapture();
            return;
        }

        if (!m_oneShot)
        {
            uint32_t currentFrame = this->currentFrame();
            currentFrame++;
            if (currentFrame >= numFrames)
            {
                if (loop)
                {
                    currentFrame = 0;
                    firstFrame = true;
                    m_reader.seek (currentFrame);
                }
                else
                {
                    stopCapture();
                }
            }
            else
            {
                std::lock_guard<std::mutex> lck (m_playbackMutex);
                m_reader.seek (currentFrame);
            }
        }

        if (m_oneShot)
        {
            m_oneShot = false;
        }
    }
}

void CameraPlayback::internalCallback (std::vector<ICapturedRawFrame *> &frames,
                                       const royale::usecase::UseCaseDefinition &definition,
                                       std::unique_ptr<const CapturedUseCase> capturedCase,
                                       royale::StreamId streamId)
{
    if (m_captureListener)
    {
        try
        {
            m_captureListener->captureCallback (frames, definition, streamId, std::move (capturedCase));
        }
        catch (...)
        {
            LOG (DEBUG) << "Exception during captureCallback";
            m_eventForwarder.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING, "Exception during captureCallback");
        }
    }
}

void CameraPlayback::saveProcessingParameters (royale::StreamId streamId)
{
    // Save the complete parameter map, the vector might only contain some
    // of the parameters
    m_processing->getProcessingParameters (m_userParameters[streamId], streamId);
}

CameraStatus CameraPlayback::setupListeners()
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    if (m_callbackData == (uint16_t) CallbackData::Depth)
    {
        if (m_isInitialized && !m_processing->isReadyToProcessDepthData())
        {
            return CameraStatus::NO_CALIBRATION_DATA;
        }

        LOG (DEBUG) << "Setup listeners for DEPTH data";

        m_processing->registerDataListeners (m_listeners);
    }
    else if (m_callbackData == (uint16_t) CallbackData::Raw)
    {
        LOG (DEBUG) << "Setup listeners for RAW data";

        m_processing->registerDataListeners (m_listeners);
    }
    else if (m_callbackData == (uint16_t) CallbackData::Intermediate)
    {
        if (m_isInitialized && !m_processing->isReadyToProcessDepthData())
        {
            return CameraStatus::NO_CALIBRATION_DATA;
        }

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
        return CameraStatus::LOGIC_ERROR;
    }

    return CameraStatus::SUCCESS;
}

CameraStatus CameraPlayback::getCameraInfo (
    royale::Vector<royale::Pair<royale::String, royale::String>> &camInfo) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    camInfo.clear();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getStreams (royale::Vector<uint16_t> &streams) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    std::lock_guard<std::recursive_mutex> lckIds (m_currentUseCaseMutex);
    streams = m_streams;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::writeCalibrationToFlash() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::writeDataToFlash (const royale::Vector<uint8_t> &data) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::writeDataToFlash (const royale::String &filename) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getExposureLimits (royale::Pair<uint32_t, uint32_t> &exposureLimits, royale::StreamId streamId) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getExposureGroups (royale::Vector< royale::String > &exposureGroups) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getExposureLimits (const String &exposureGroup, royale::Pair<uint32_t, uint32_t> &exposureLimits) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setExposureTime (const String &exposureGroup, uint32_t exposureTime) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::initialize (const royale::String &initUseCase) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setExposureTime (uint32_t exposureTime, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setExposureTimes (const Vector<uint32_t> &exposureTimes, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setExposureForGroups (const Vector<uint32_t> &exposureTimes) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setExposureMode (ExposureMode exposureMode, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getExposureMode (royale::ExposureMode &exposureMode, royale::StreamId streamId) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::writeRegisters (const royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

royale::CameraStatus CameraPlayback::readRegisters (royale::Vector<royale::Pair<royale::String, uint64_t>> &registers) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setFrameRate (uint16_t framerate) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getFrameRate (uint16_t &frameRate) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    frameRate = 0u;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getMaxFrameRate (uint16_t &maxFrameRate) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    maxFrameRate = 0u;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::startRecording (const String &fileName,
        uint32_t numberOfFrames,
        uint32_t frameSkip,
        uint32_t msSkip) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::stopRecording() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setDutyCycle (double dutyCycle, uint16_t index) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

uint16_t CameraPlayback::getFileVersion()
{
    return m_reader.getFileVersion();
}

void CameraPlayback::readFrame (std::vector<std::vector<uint16_t>> &frames,
                                std::unique_ptr<royale::usecase::UseCaseDefinition> &definition,
                                std::chrono::milliseconds &timestamp,
                                float &illuminationTemperature,
                                std::vector<uint32_t> &capturedExposureTimes,
                                royale::ProcessingParameterMap &parameterMap,
                                std::vector<std::pair<std::string, std::vector<uint8_t>>> &additionalData,
                                royale::StreamId &streamId)
{
    RecordUseCaseDefinition recordDefinition;

    std::vector <std::vector<uint16_t>> imageData;
    std::vector <std::vector<uint16_t>> pseudoData;
    std::vector <royale_processingparameter_v3> processingParameters;

    m_reader.get (imageData, pseudoData, &recordDefinition.frameHeader, recordDefinition.streams, recordDefinition.frameGroups,
                  recordDefinition.exposureGroups, recordDefinition.rawFrameSets, processingParameters, additionalData);

    {
        std::lock_guard<std::recursive_mutex> lckIds (m_currentUseCaseMutex);
        m_streams.clear();
        for (auto curStream : recordDefinition.streams)
        {
            m_streams.push_back (curStream.streamId);
        }
    }

    streamId = recordDefinition.frameHeader.curStreamId;

    frames.resize (recordDefinition.frameHeader.numRawFrames);
    for (auto i = 0; i < recordDefinition.frameHeader.numRawFrames; ++i)
    {
        frames.at (i).resize (pseudoData[i].size () + imageData[i].size ());
    }

    recordDefinition.rawFrameSets.resize (recordDefinition.frameHeader.numRawFrameSets);

    timestamp = std::chrono::milliseconds (recordDefinition.frameHeader.timestamp);
    illuminationTemperature = recordDefinition.frameHeader.illuTemperature;
    capturedExposureTimes.resize (recordDefinition.frameHeader.numRawFrameSets);

    for (auto j = 0u; j < recordDefinition.frameHeader.numRawFrames; ++j)
    {
        memcpy (&frames[j][0], &pseudoData[j][0], pseudoData[j].size() * sizeof (uint16_t));
        memcpy (&frames[j][pseudoData[j].size()], &imageData[j][0], imageData[j].size() * sizeof (uint16_t));
    }

    definition = common::makeUnique<UseCaseRecord> (recordDefinition);

    for (auto i = 0u; i < recordDefinition.frameHeader.numRawFrameSets; ++i)
    {
        capturedExposureTimes[i] = recordDefinition.rawFrameSets[i].capturedExpTime;
    }

    uint16_t numFreqs = 0u;
    auto rfsIndexes = definition->getRawFrameSetIndices (streamId, 0u);
    for (auto curIndex : rfsIndexes)
    {
        if (definition->getRawFrameSets() [curIndex].isModulated())
        {
            numFreqs++;
        }
    }

    if (recordDefinition.frameHeader.numParameters > 0)
    {
        for (auto i = 0u; i < recordDefinition.frameHeader.numParameters; ++i)
        {
            ProcessingFlag curFlag = static_cast<ProcessingFlag> (processingParameters[i].processingFlag);
            VariantType curType = static_cast<VariantType> (processingParameters[i].dataType);
            parameterMap[curFlag] = Variant (curType, processingParameters[i].value);
        }
    }

    if (numFreqs == 1)
    {
        // Old recordings don't have this flag set which causes the configuration of Spectre
        // to break
        parameterMap[ProcessingFlag::UseFilter2Freq_Bool] = false;
    }
}

royale::CameraStatus CameraPlayback::shiftLensCenter (int16_t tx, int16_t ty) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

royale::CameraStatus CameraPlayback::getLensCenter (uint16_t &x, uint16_t &y) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::registerEventListener (royale::IEventListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    CameraDeviceBase::registerEventListener (listener);
    m_eventForwarder.setEventListener (listener);
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::unregisterEventListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    CameraDeviceBase::unregisterEventListener();
    m_eventForwarder.setEventListener (nullptr);
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getNumberOfStreams (const royale::String &name, uint32_t &nrStreams) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setExternalTrigger (bool useExternalTrigger) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return CameraStatus::NOT_IMPLEMENTED;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::setProcessingParameters (const royale::ProcessingParameterVector &parameters, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    return CameraDeviceBase::setProcessingParameters (parameters, streamId);
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraPlayback::getProcessingParameters (royale::ProcessingParameterVector &parameters, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    std::lock_guard<std::mutex> lck (m_playbackMutex);
    return CameraDeviceBase::getProcessingParameters (parameters, streamId);
}
ROYALE_API_EXCEPTION_SAFE_END

