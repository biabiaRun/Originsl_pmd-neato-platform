/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <processing/Processing.hpp>
#include <common/NarrowCast.hpp>
#include <common/exceptions/CalibrationDataNotFound.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>

using namespace royale::processing;
using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;
using namespace royale::processing;
using namespace royale;

Processing::Processing (IFrameCaptureReleaser *releaser,
                        IRefineExposureTime *refineExposureTime)
    : m_exposureListener (nullptr),
      m_releaser (releaser),
      m_refineExposureTime (refineExposureTime)
{
}

Processing::~Processing()
{
    // Acquire the lock so we're not in the middle of a callback
    std::lock_guard<std::mutex> lock (m_listenerMutex);
}

void Processing::captureCallback (std::vector<ICapturedRawFrame *> &frames,
                                  const UseCaseDefinition &definition,
                                  royale::StreamId streamId,
                                  std::unique_ptr<const CapturedUseCase> capturedCase)
{
    if (!m_listeners.depthDataListener &&
            !m_listeners.depthImageListener &&
            !m_listeners.sparsePointCloudListener &&
            !m_listeners.irImageListener &&
            !m_listeners.extendedListener)
    {
        m_releaser->releaseCapturedFrames (frames);
        return;
    }

    m_depthDataBuffer.streamId = streamId;

    m_depthDataBuffer.depthData->timeStamp = capturedCase->getTimestamp();

    const auto &expoGroupIndices = definition.getExposureIndicesForStream (streamId);

    const royale::Vector<uint32_t> expTimes = capturedCase->getExposureTimes();

    // The capturedCase contains the exposure times in exposure group order for all streams.
    // We have to transform them back to raw frame set order and only use the ones for the
    // current stream.
    royale::Vector<uint32_t> capturedTimes;
    for (auto expoIdx : expoGroupIndices)
    {
        capturedTimes.push_back (expTimes[expoIdx]);
    }

    m_depthDataBuffer.depthData->exposureTimes = capturedTimes; // copy exposureTimes

    const auto &rawFrameSets = definition.getRawFrameSets();

    royale::Vector<uint32_t> modulationFrequencies;
    const auto &frameSetIdxs = definition.getRawFrameSetIndices (streamId, 0);
    for (const auto frameSetIdx : frameSetIdxs)
    {
        const auto &rawFrameSet = rawFrameSets.at (frameSetIdx);
        modulationFrequencies.push_back (rawFrameSet.modulationFrequency);
    }

    if (m_listeners.extendedListener)
    {
        definition.getImage (m_depthDataBuffer.rawData->width, m_depthDataBuffer.rawData->height);
        m_depthDataBuffer.rawData->rawData.resize (frames.size());
        m_depthDataBuffer.rawData->illuminationTemperature = capturedCase->getIlluminationTemperature();

        m_depthDataBuffer.rawData->streamId = streamId;
        m_depthDataBuffer.rawData->modulationFrequencies = modulationFrequencies;
        m_depthDataBuffer.rawData->exposureTimes = capturedTimes;

        m_depthDataBuffer.intermediateData->modulationFrequencies = modulationFrequencies;
        m_depthDataBuffer.intermediateData->exposureTimes = m_depthDataBuffer.rawData->exposureTimes;

        // assign data pointers
        for (size_t i = 0; i < frames.size(); i++)
        {
            m_depthDataBuffer.rawData->rawData[i] = frames[i]->getImageData();
        }

        royale::Vector<size_t> rawFrameCount;
        m_depthDataBuffer.rawData->illuminationEnabled.clear();
        m_depthDataBuffer.rawData->phaseAngles.clear();
        for (const auto &rawSetIndex : frameSetIdxs)
        {
            uint8_t illuEnabled = rawFrameSets.at (rawSetIndex).dutyCycle != RawFrameSet::DutyCycle::DC_0 ? 1 : 0;
            rawFrameCount.push_back (rawFrameSets.at (rawSetIndex).countRawFrames());
            for (const auto &curPhaseAngle : rawFrameSets.at (rawSetIndex).getPhaseAngles())
            {
                m_depthDataBuffer.rawData->phaseAngles.push_back (curPhaseAngle);
                m_depthDataBuffer.rawData->illuminationEnabled.push_back (illuEnabled);
            }
        }
        m_depthDataBuffer.rawData->rawFrameCount = rawFrameCount;

        const auto &expoGroups = definition.getExposureGroups();
        royale::Vector<royale::String> exposureGroupNames;
        for (auto expoIdx : expoGroupIndices)
        {
            exposureGroupNames.push_back (expoGroups[expoIdx].m_name);
        }
        m_depthDataBuffer.rawData->exposureGroupNames = exposureGroupNames;

        // get timestamp
        m_depthDataBuffer.rawData->timeStamp = capturedCase->getTimestamp();
    }

    std::vector<uint32_t> newExposureTimes;

    bool dataWasProcessed = false;
    if (isReadyToProcessDepthData())
    {
        std::lock_guard<std::mutex> lock (m_calcMutex);
        try
        {
            processFrame (frames, std::move (capturedCase), m_depthDataBuffer, capturedTimes, newExposureTimes);
            dataWasProcessed = true;
        }
        catch (...)
        {
            LOG (WARN) << "There was a problem processing the data";
        }
    }

    {
        std::lock_guard<std::mutex> lock (m_listenerMutex);

        if (m_listeners.extendedListener)
        {
            if (dataWasProcessed)
            {
                m_depthDataBuffer.extendedData->setDepthData (m_depthDataBuffer.depthData.get());
                m_depthDataBuffer.extendedData->setIntermediateData (m_depthDataBuffer.intermediateData.get());
            }
            m_depthDataBuffer.extendedData->setRawData (m_depthDataBuffer.rawData.get());
            m_listeners.extendedListener->onNewData (m_depthDataBuffer.extendedData.get());

            m_releaser->releaseCapturedFrames (frames);
        }
        else
        {
            m_releaser->releaseCapturedFrames (frames);

            if (dataWasProcessed)
            {
                if (m_listeners.depthDataListener)
                {
                    m_listeners.depthDataListener->onNewData (m_depthDataBuffer.depthData.get());
                }
                if (m_listeners.depthImageListener)
                {
                    m_listeners.depthImageListener->onNewData (m_depthDataBuffer.depthImage.get());
                }
                if (m_listeners.sparsePointCloudListener)
                {
                    m_listeners.sparsePointCloudListener->onNewData (m_depthDataBuffer.sparsePointCloud.get());
                }
                if (m_listeners.irImageListener)
                {
                    m_listeners.irImageListener->onNewData (m_depthDataBuffer.irImage.get());
                }
            }
        }

        if (dataWasProcessed &&
                m_exposureListener != nullptr &&
                newExposureTimes.size() > m_exposureTimeIndex[streamId] &&
                newExposureTimes[m_exposureTimeIndex[streamId]] > 0)
        {
            auto newExpo = newExposureTimes[m_exposureTimeIndex[streamId]];
            if (newExpo < m_exposureLimits[streamId].first)
            {
                newExpo = m_exposureLimits[streamId].first;
            }
            if (newExpo > m_exposureLimits[streamId].second)
            {
                newExpo = m_exposureLimits[streamId].second;
            }

            if (newExpo != m_depthDataBuffer.depthData->exposureTimes[m_exposureTimeIndex[streamId]])
            {
                m_exposureListener->onNewExposure (newExpo, streamId);
            }
        }
    }
}

void Processing::releaseAllFrames ()
{
    // with a synchronous implementation of captureCallback that calls
    // releaseCapturedFrame() before returning, this can return immediately
}

void Processing::registerExposureListener (royale::IExposureListener2 *exposureListener)
{
    std::lock_guard<std::mutex> lock (m_listenerMutex);
    m_exposureListener = exposureListener;
}

void Processing::unregisterExposureListener ()
{
    std::lock_guard<std::mutex> lock (m_listenerMutex);
    m_exposureListener = nullptr;
}

VerificationStatus Processing::verifyUseCase (const UseCaseDefinition &useCase)
{
    return VerificationStatus::UNDEFINED;
}

void Processing::setCameraName (const String &cameraName)
{
    m_cameraName = String::toStdString (cameraName);
}

bool Processing::hasLensCenterCalibration() const
{
    return false;
}

void Processing::setProcessingParameters (const ProcessingParameterMap &parameters,
        const royale::StreamId streamId)
{
    if (m_parameters[streamId].empty())
    {
        m_parameters[streamId] = parameters;
    }
    else
    {
        for (auto &oldParameter : m_parameters[streamId])
        {
            auto newParameter = parameters.find (oldParameter.first);
            if (newParameter != parameters.end() &&
                    newParameter->second != oldParameter.second &&
                    newParameter->second.variantType() == oldParameter.second.variantType())
            {
                oldParameter.second = newParameter->second;
            }
        }
    }
}

void Processing::getProcessingParameters (ProcessingParameterMap &parameters,
        const royale::StreamId streamId)
{
    parameters = m_parameters[streamId];
}

void Processing::setUseCase (const UseCaseDefinition &useCase)
{
    std::lock_guard<std::mutex> lock (m_listenerMutex);
    const auto &rawFrameSets = useCase.getRawFrameSets();
    const auto &streamIds = useCase.getStreamIds();

    m_parameters.clear();
    m_exposureTimeIndex.clear();

    for (const auto &streamId : streamIds)
    {
        if (useCase.getFrameGroupCount (streamId) == 0u)
        {
            continue;
        }

        const auto &frameSetIdxs = useCase.getRawFrameSetIndices (streamId, 0);
        bool expoIndexSet = false;
        size_t idx = 0;
        for (const auto frameSetIdx : frameSetIdxs)
        {
            const auto &rawFrameSet = rawFrameSets.at (frameSetIdx);
            if (!expoIndexSet && rawFrameSet.isModulated())
            {
                m_exposureTimeIndex[streamId] = static_cast<uint32_t> (idx);
                expoIndexSet = true;
            }
            ++idx;
        }
        m_exposureLimits[streamId] = useCase.getExposureLimits (streamId);
    }
}


void Processing::registerDataListeners (const DataListeners &listeners)
{
    std::lock_guard<std::mutex> lock (m_listenerMutex);
    m_listeners = listeners;
}

uint32_t Processing::getRefinedExposureTime (const uint32_t exposureTime,
        const royale::Pair<uint32_t, uint32_t> &exposureLimits)
{
    if (m_refineExposureTime)
    {
        return m_refineExposureTime->getRefinedExposureTime (exposureTime, exposureLimits);
    }

    // We don't know how to refine it -> return the original value
    return exposureTime;
}

royale::Vector<royale::Pair<royale::String, royale::String>> Processing::getProcessingInfo()
{
    royale::Vector<royale::Pair<royale::String, royale::String>> procInfo;
    procInfo.emplace_back (std::make_pair ("PROCESSING_NAME", getProcessingName()));
    procInfo.emplace_back (std::make_pair ("PROCESSING_VERSION", getProcessingVersion()));

    return procInfo;
}
