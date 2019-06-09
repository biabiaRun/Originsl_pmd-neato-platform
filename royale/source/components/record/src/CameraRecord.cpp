/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <record/CameraRecord.hpp>
#include <royale/Vector.hpp>
#include <royale/String.hpp>

#include <NarrowCast.hpp>
#include <factory/ImagerFactory.hpp>

using namespace royale::record;
using namespace royale;
using namespace royale::common;
using namespace royale::collector;
using namespace royale::usecase;


CameraRecord::CameraRecord (IRecordStopListener *recordStopListener,
                            IFrameCaptureListener *rawDataListener,
                            IFrameCaptureReleaser *releaser,
                            String cameraName,
                            royale::config::ImagerType imagerType)
    : m_listener (rawDataListener),
      m_framesToRecord (0),
      m_currentFrame (0),
      m_isRecording (false),
      m_recordStopListener (recordStopListener),
      m_cameraName (cameraName),
      m_releaser (releaser)
{
    m_parameterMap.clear();

    m_imagerType = factory::ImagerFactory::getImagerTypeName (imagerType);
    m_pseudoDataInter = factory::ImagerFactory::getPseudoDataInterpreter (imagerType);
}

CameraRecord::~CameraRecord()
{
    if (m_isRecording)
    {
        stopRecord();
    }
}

void CameraRecord::startRecord (const String &filename, const std::vector<uint8_t> &calibrationData,
                                const String &imagerSerial,
                                const uint32_t numFrames, const uint32_t frameSkip, const uint32_t msSkip)
{
    // stop recording if it is already running
    stopRecord();

    {
        std::lock_guard<std::mutex> lck (m_mutex);

#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
        royale_rrf_platformtype platform = royale_rrf_platformtype::RRF_ROYALE_WINDOWS;
#elif ROYALE_TARGET_PLATFORM_LINUX
        royale_rrf_platformtype platform = royale_rrf_platformtype::RRF_ROYALE_LINUX;
#elif ROYALE_TARGET_PLATFORM_APPLE
        royale_rrf_platformtype platform = royale_rrf_platformtype::RRF_ROYALE_MAC;
#elif ROYALE_TARGET_PLATFORM_ANDROID
        royale_rrf_platformtype platform = royale_rrf_platformtype::RRF_ROYALE_ANDROID;
#endif

        // @TODO : Fill with meaningful versions
        std::vector<std::string> componentNames;
        std::vector<std::string> componentTypes;
        std::vector<std::string> componentVersions;

        m_writer.open (filename.c_str(), calibrationData, imagerSerial.c_str(), m_cameraName.c_str(),
                       m_imagerType.c_str(), m_pseudoDataInter.c_str(),
                       ROYALE_VERSION_MAJOR, ROYALE_VERSION_MINOR, ROYALE_VERSION_PATCH, ROYALE_VERSION_BUILD,
                       platform, componentNames, componentTypes, componentVersions);
        m_framesToRecord = numFrames;
        m_framesToSkip = frameSkip;
        m_framesSkipped = frameSkip;
        m_msToSkip = msSkip;
        m_lastCapture = std::chrono::milliseconds (0);
        m_currentFrame = 0;
    }
    m_isRecording = true;
}

void CameraRecord::stopRecord()
{
    bool doCallListener = false;
    if (m_isRecording)
    {
        std::lock_guard<std::mutex> lck (m_mutex);
        m_writer.close();
        doCallListener = true;
        m_isRecording = false;
    }
    if (m_recordStopListener && doCallListener)
    {
        // needs to be called with m_mutex unlocked, due to possible deadlock with conveyance thread
        m_recordStopListener->onRecordingStopped (m_currentFrame);
    }
}

bool CameraRecord::isRecording()
{
    return m_isRecording;
}

bool CameraRecord::setFrameCaptureListener (royale::collector::IFrameCaptureListener *captureListener)
{
    if (isRecording())
    {
        return false;
    }
    m_listener = captureListener;
    return true;
}

void CameraRecord::captureCallback (std::vector<ICapturedRawFrame *> &frames,
                                    const royale::usecase::UseCaseDefinition &definition,
                                    royale::StreamId streamId,
                                    std::unique_ptr<const CapturedUseCase> capturedCase)
{
    {
        std::lock_guard<std::mutex> lck (m_mutex);
        if (m_isRecording)
        {
            bool recordThisFrame = true;
            if (m_framesSkipped < m_framesToSkip)
            {
                recordThisFrame = false;
                m_framesSkipped++;
            }
            // captured usecase timestamps are in unit microseconds
            std::chrono::milliseconds timeSkipped = std::chrono::duration_cast<std::chrono::milliseconds> (capturedCase->getTimestamp()) - m_lastCapture;
            if (timeSkipped.count() < m_msToSkip)
            {
                recordThisFrame = false;
            }

            if (recordThisFrame)
            {
                m_currentFrame++;

                m_framesSkipped = 0;
                m_lastCapture = std::chrono::duration_cast<std::chrono::milliseconds> (capturedCase->getTimestamp());

                putFrame (frames, definition, streamId, *capturedCase, m_parameterMap[streamId]);
            }
        }
    }

    if (m_isRecording && m_framesToRecord && (m_currentFrame == m_framesToRecord))
    {
        stopRecord();
    }


    if (m_listener)
    {
        m_listener->captureCallback (frames, definition, streamId, std::move (capturedCase));
    }
    else
    {
        if (m_releaser)
        {
            m_releaser->releaseCapturedFrames (frames);
        }
    }
}

void CameraRecord::releaseAllFrames ()
{
    // captureCallback is synchronous w.r.t calling releaseCapturedFrame(),
    // so this doesn't need to block
}

void CameraRecord::setProcessingParameters (const ProcessingParameterVector &parameters, const royale::StreamId streamId)
{
    std::lock_guard<std::mutex> lck (m_mutex);
    m_parameterMap[streamId] = ProcessingParameterVector::toStdMap (parameters);
}

void CameraRecord::putFrame (const std::vector<ICapturedRawFrame *> &frames,
                             const UseCaseDefinition &definition,
                             const royale::StreamId streamId,
                             const CapturedUseCase &capturedCase,
                             const ProcessingParameterMap &parameterMap)
{
    royale_frameheader_v3 frameHeader;

    // Fill frame header
    frameHeader.targetFrameRate = definition.getTargetRate();
    definition.getImage (frameHeader.numColumns, frameHeader.numRows);

    frameHeader.numRawFrames = narrow_cast<uint16_t> (frames.size());
    frameHeader.numRawFrameSets = narrow_cast<uint16_t> (definition.getRawFrameSets().size());

    // we need to convert to milliseconds unit to stay compatible with older files
    frameHeader.timestamp = std::chrono::duration_cast<std::chrono::milliseconds> (capturedCase.getTimestamp()).count();
    frameHeader.illuTemperature = capturedCase.getIlluminationTemperature();

    frameHeader.numParameters = narrow_cast<uint32_t> (parameterMap.size());

    auto useCaseName = definition.getTypeName(); // \todo ROYAL-3267 should this be UseCase.getName() instead of UseCaseDef.get...()?
    useCaseName.resize (ROYALE_FILEHEADER_V3_USE_CASE_LENGTH);
    memcpy (&frameHeader.useCaseName, useCaseName.c_str(), ROYALE_FILEHEADER_V3_USE_CASE_LENGTH);

    frameHeader.curStreamId = streamId;
    frameHeader.numStreams = static_cast<uint16_t> (definition.getStreamIds().size());

    frameHeader.numFrameGroups = 0;

    std::vector<royale_streamheader_v3> streamHeaders;
    std::vector<royale_framegroupheader_v3> frameGroupHeaders;

    auto streamIds = definition.getStreamIds();

    streamHeaders.resize (streamIds.size());

    for (auto i = 0u; i < streamIds.size(); ++i)
    {
        const auto &stream = definition.getStream (streamIds[i]);

        streamHeaders[i].streamId = streamIds[i];
        streamHeaders[i].numFrameGroups = static_cast<uint16_t> (stream->m_frameGroups.size());

        for (auto j = 0u; j < streamHeaders[i].numFrameGroups; ++j)
        {
            royale_framegroupheader_v3 fgHeader;
            fgHeader.numRawFrameSets = static_cast<uint16_t> (stream->m_frameGroups[j].m_frameSetIds.size());
            for (auto k = 0u; k < fgHeader.numRawFrameSets; ++k)
            {
                fgHeader.rawFrameSetIdxs[k] = static_cast<uint16_t> (stream->m_frameGroups[j].m_frameSetIds[k]);
            }

            frameGroupHeaders.push_back (fgHeader);
            streamHeaders[i].frameGroupIdxs[j] = frameHeader.numFrameGroups;
            frameHeader.numFrameGroups++;
        }
    }

    std::vector<royale_exposuregroupheader_v3> exposureGroupHeaders;
    const auto &exposureGroups = definition.getExposureGroups();
    frameHeader.numExposureGroups = static_cast<uint16_t> (exposureGroups.size());
    exposureGroupHeaders.resize (exposureGroups.size ());

    for (auto i = 0u; i < exposureGroups.size(); ++i)
    {
        std::string tmpName (exposureGroups[i].m_name.c_str());
        tmpName.resize (ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH);
        memcpy (&exposureGroupHeaders[i].exposureGroupName[0], tmpName.c_str(), ROYALE_FILEHEADER_V3_EXPOSUREGROUP_NAME_LENGTH);

        exposureGroupHeaders[i].exposureMin = exposureGroups[i].m_exposureLimits.first;
        exposureGroupHeaders[i].exposureMax = exposureGroups[i].m_exposureLimits.second;
        exposureGroupHeaders[i].exposureTime = exposureGroups[i].m_exposureTime;
    }

    std::vector<royale_processingparameter_v3> processingParameters;

    // Write parameters
    for (auto curParam = parameterMap.begin(); curParam != parameterMap.end(); ++curParam)
    {
        royale_processingparameter_v3 parameterHeader;
        parameterHeader.processingFlag = static_cast<uint32_t> (curParam->first);
        parameterHeader.dataType = static_cast<uint8_t> (curParam->second.variantType());
        parameterHeader.value = curParam->second.getData();

        processingParameters.push_back (parameterHeader);
    }

    std::vector<royale_rawframesetheader_v3> rawFrameSetHeaders;
    std::vector<const uint16_t *> imageData;
    std::vector<const uint16_t *> pseudoData;

    for (auto i = 0u; i < frameHeader.numRawFrameSets; ++i)
    {
        const RawFrameSet &rawFrameSet = definition.getRawFrameSets().at (i);
        royale_rawframesetheader_v3 rawFrameSetHeader;

        rawFrameSetHeader.modFreq = rawFrameSet.modulationFrequency;
        rawFrameSetHeader.capturedExpTime = capturedCase.getExposureTimes() [i];
        rawFrameSetHeader.phaseDefinition = static_cast<uint8_t> (rawFrameSet.phaseDefinition);
        rawFrameSetHeader.numRawFrames = narrow_cast<uint16_t> (rawFrameSet.countRawFrames());
        rawFrameSetHeader.dutyCycle = static_cast<uint16_t> (rawFrameSet.dutyCycle);
        rawFrameSetHeader.alignment = static_cast<uint16_t> (rawFrameSet.alignment);
        rawFrameSetHeader.exposureGroupIdx = static_cast<uint16_t> (rawFrameSet.exposureGroupIdx);
        rawFrameSetHeader.eyeSafetyGap = rawFrameSet.tEyeSafety;

        rawFrameSetHeaders.push_back (rawFrameSetHeader);
    }

    for (uint16_t i = 0; i < frames.size(); ++i)
    {
        ICapturedRawFrame *capturedRawFrame = frames.at (i);

        imageData.push_back (capturedRawFrame->getImageData());
        pseudoData.push_back (capturedRawFrame->getPseudoData());
    }

    // Currently Royale doesn't output additional frame data
    std::vector <royale_additionaldata_v3> additionalData;
    frameHeader.numAdditionalData = static_cast<uint32_t> (additionalData.size());

    m_writer.put (imageData, pseudoData, &frameHeader, streamHeaders, frameGroupHeaders, exposureGroupHeaders,
                  rawFrameSetHeaders, processingParameters, additionalData);
}

void CameraRecord::resetParameters()
{
    m_parameterMap.clear();
}
