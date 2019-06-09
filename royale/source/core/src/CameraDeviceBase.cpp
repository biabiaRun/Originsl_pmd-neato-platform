/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <device/CameraDeviceBase.hpp>
#include <common/FileSystem.hpp>
#include <common/exceptions/APIExceptionHandling.hpp>
#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale;
using namespace royale::processing;
using namespace royale::common;
using namespace royale::device;
using namespace royale::config;
using namespace royale::usecase;

CameraDeviceBase::CameraDeviceBase (CameraAccessLevel level, const std::string &id,
                                    const std::string &cameraName, std::shared_ptr<IProcessing> processing,
                                    royale::CallbackData cbData) :
    m_id (id),
    m_cameraName (cameraName),
    m_currentUseCaseName ("NOT_VALID"),
    m_callbackData ( (uint16_t) cbData),
    m_accessLevel (level),
    m_recording (nullptr),
    m_recordStopListener (nullptr),
    m_processing (processing),
    m_exposureLimits (0, 0),
    m_exposureListener1 (nullptr),
    m_exposureListener2 (nullptr),
    m_currentUseCaseDefinition (nullptr),
    m_isPartiallyInitialized (false),
    m_isInitialized (false)
{

}

CameraStatus CameraDeviceBase::getId (String &id) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    id = m_id;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getAccessLevel (CameraAccessLevel &accessLevel) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    accessLevel = m_accessLevel;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getCameraName (String &cameraName) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    cameraName = m_cameraName;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getCurrentUseCase (royale::String &useCase) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    useCase = m_currentUseCaseName;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getUseCases (Vector<String> &useCases) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    useCases = m_supportedUseCaseNames;
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getLensParameters (LensParameters &param) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    try
    {
        m_processing->getLensParameters (param);
    }
    catch (Exception &e)
    {
        return e.getStatus();
    }
    catch (...)
    {
        return CameraStatus::DATA_NOT_FOUND;
    }
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::isCalibrated (bool &calibrated) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    calibrated = m_processing->hasCalibrationData();
    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::setCallbackData (CallbackData cbData) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return setCallbackData ( (uint16_t) cbData);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerDepthImageListener (IDepthImageListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return setSingleListener<IDepthImageListener> (listener, &m_listeners.depthImageListener);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterDepthImageListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_listeners.depthImageListener = nullptr;
    return setupListeners();
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerSparsePointCloudListener (ISparsePointCloudListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return setSingleListener<ISparsePointCloudListener> (listener, &m_listeners.sparsePointCloudListener);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterSparsePointCloudListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_listeners.sparsePointCloudListener = nullptr;
    return setupListeners();
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerIRImageListener (IIRImageListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    return setSingleListener<IIRImageListener> (listener, &m_listeners.irImageListener);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterIRImageListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_listeners.irImageListener = nullptr;
    return setupListeners();
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerDataListenerExtended (IExtendedDataListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    return setSingleListener<IExtendedDataListener> (listener, &m_listeners.extendedListener);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterDataListenerExtended() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    m_listeners.extendedListener = nullptr;
    return setupListeners();
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerDataListener (IDepthDataListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_processing->isReadyToProcessDepthData())
    {
        return CameraStatus::NO_CALIBRATION_DATA;
    }

    return setSingleListener<IDepthDataListener> (listener, &m_listeners.depthDataListener);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterDataListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_listeners.depthDataListener = nullptr;
    return setupListeners();
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerEventListener (royale::IEventListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_eventListener = listener;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterEventListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_eventListener = nullptr;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerRecordListener (IRecordStopListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_recordStopListener = listener;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterRecordListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    m_recordStopListener = nullptr;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::setCalibrationData (const String &filename) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    if (!fileexists (filename))
    {
        return CameraStatus::FILE_NOT_FOUND;
    }

    Vector<uint8_t> data;

    if (!readFileToVector (filename, data))
    {
        return CameraStatus::DATA_NOT_FOUND;
    }

    return setCalibrationData (data);
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::setCalibrationData (const Vector<uint8_t> &data) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    m_calibrationData = data;
    if (m_callbackData != (uint16_t) CallbackData::Raw)
    {
        try
        {
            m_processing->setCalibrationData (m_calibrationData.toStdVector());
        }
        catch (const Exception &)
        {
            return CameraStatus::CALIBRATION_DATA_ERROR;
        }
    }

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getCalibrationData (Vector<uint8_t> &data) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    data = m_calibrationData;

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerExposureListener (IExposureListener *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    std::lock_guard<std::recursive_mutex> lock (m_exposureListenerMutex);
    m_exposureListener1 = listener;
    m_exposureListener2 = nullptr;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::registerExposureListener (IExposureListener2 *listener) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    std::lock_guard<std::recursive_mutex> lock (m_exposureListenerMutex);
    m_exposureListener1 = nullptr;
    m_exposureListener2 = listener;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::unregisterExposureListener() ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    std::lock_guard<std::recursive_mutex> lock (m_exposureListenerMutex);
    m_exposureListener1 = nullptr;
    m_exposureListener2 = nullptr;
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

void CameraDeviceBase::callExposureListeners (const UseCaseDefinition &definition)
{
    // tell the listeners about the current exposure times
    std::lock_guard<std::recursive_mutex> lock (m_exposureListenerMutex);
    if (m_exposureListener1 != nullptr ||
            m_exposureListener2 != nullptr)
    {
        auto rawFrameSets = definition.getRawFrameSets();
        auto streamIds = definition.getStreamIds();
        auto &exposureGroups = definition.getExposureGroups();
        // The "old" exposureListener1 doesn't know about streams, so it will
        // only get updates for the first stream
        bool expo1ListenerCalledForFirstStream = false;
        for (const auto &streamId : streamIds)
        {
            const auto &frameSetIdxs = definition.getRawFrameSetIndices (streamId, 0);
            for (const auto frameSetIdx : frameSetIdxs)
            {
                const auto &rawFrameSet = rawFrameSets.at (frameSetIdx);
                if (rawFrameSet.isModulated())
                {
                    if (m_exposureListener1 != nullptr &&
                            !expo1ListenerCalledForFirstStream)
                    {
                        m_exposureListener1->onNewExposure (exposureGroups[rawFrameSet.exposureGroupIdx].m_exposureTime);
                        expo1ListenerCalledForFirstStream = true;
                    }
                    if (m_exposureListener2 != nullptr)
                    {
                        m_exposureListener2->onNewExposure (exposureGroups[rawFrameSet.exposureGroupIdx].m_exposureTime, streamId);
                    }
                    break;
                }
            }

        }
    }
}

void CameraDeviceBase::normalizeStreamId (StreamId &streamId) const
{
    if (!m_isInitialized)
    {
        throw RuntimeError ("runtime error", "not initialized", CameraStatus::DEVICE_NOT_INITIALIZED);
    }

    std::lock_guard<std::recursive_mutex> lock (m_currentUseCaseMutex);

    if (streamId == 0)
    {
        if (m_streams.size() != 1)
        {
            throw RuntimeError ("runtime error", "streamId 0 not valid for mixed mode", CameraStatus::INVALID_VALUE);
        }
        streamId = m_streams.front();
    }
    else if (!m_streams.contains (streamId))
    {
        throw RuntimeError ("runtime error", "streamId not found", CameraStatus::INVALID_VALUE);
    }
}

royale::CameraStatus CameraDeviceBase::setFilterLevel (royale::FilterLevel level, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (level == FilterLevel::Custom)
    {
        return CameraStatus::INVALID_VALUE;
    }

    if (!m_processing->isReadyToProcessDepthData())
    {
        return CameraStatus::NO_CALIBRATION_DATA;
    }

    normalizeStreamId (streamId);

    try
    {
        m_processing->setFilterLevel (level, streamId);
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Error setting filter level : " << e.what();
        return CameraStatus::INVALID_VALUE;
    }

    saveProcessingParameters (streamId);

    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

royale::CameraStatus CameraDeviceBase::getFilterLevel (royale::FilterLevel &level, royale::StreamId streamId) const ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    if (!m_processing->isReadyToProcessDepthData())
    {
        return CameraStatus::NO_CALIBRATION_DATA;
    }

    normalizeStreamId (streamId);

    try
    {
        level = m_processing->getFilterLevel (streamId);
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Error retrieving filter level : " << e.what();
        return CameraStatus::INVALID_VALUE;
    }

    return CameraStatus::SUCCESS;
}
ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::setProcessingParameters (const ProcessingParameterVector &parameters, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    normalizeStreamId (streamId);

    try
    {
        m_processing->setProcessingParameters (ProcessingParameterVector::toStdMap (parameters), streamId);
    }
    catch (Exception &e)
    {
        LOG (ERROR) << "Error setting processing parameters : " << e.what();
        return CameraStatus::RUNTIME_ERROR;
    }

    saveProcessingParameters (streamId);

    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END

CameraStatus CameraDeviceBase::getProcessingParameters (ProcessingParameterVector &parameters, royale::StreamId streamId) ROYALE_API_EXCEPTION_SAFE_BEGIN
{
    ROYALE_ACCESS_LEVEL_CHECK (CameraAccessLevel::L2)

    normalizeStreamId (streamId);

    ProcessingParameterMap parameterMap;
    m_processing->getProcessingParameters (parameterMap, streamId);

    parameters = ProcessingParameterVector::fromStdMap<ProcessingFlag, Variant> (parameterMap);
    return CameraStatus::SUCCESS;
} ROYALE_API_EXCEPTION_SAFE_END
