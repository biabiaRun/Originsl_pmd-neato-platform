/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/Vector.hpp>

#include <processing/ProcessingSpectre.hpp>
#include <processing/ParameterMapping.hpp>

#include <royale/ProcessingFlag.hpp>

#include <usecase/HardcodedMaxStreams.hpp>

#include <common/exceptions/CalibrationDataNotFound.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/FileNotFound.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/MakeUnique.hpp>
#include <NarrowCast.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <RoyaleLogger.hpp>

#include <FileSystem.hpp>

#include <cstring>
#include <stdint.h>
#include <limits.h>
#include <algorithm>
#include <set>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <iostream>
#include <ostream>
#include <fstream>

using namespace royale::processing;
using namespace royale;
using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;

using namespace spectre;
using namespace spectre::common;

namespace
{
    template<typename T>
    bool setParameter (IConfiguration &config, ParameterKey key, T val)
    {
        auto par = config.getParameterByKey (key);
        if (par.isValid() && par.setValue (ParameterVariant (val)))
        {
            return config.setParameter (par);
        }

        return false;
    }

    template<typename T>
    void setParameterOrFail (IConfiguration &config, ParameterKey key, T val)
    {
        if (!setParameter (config, key, val))
        {
            throw RuntimeError ("Could not set parameter");
        }
    }

    std::unique_ptr<ISpectre> createSpectre (std::vector<uint8_t> &calibrationData)
    {
        ISpectre *spectre = nullptr;

        auto spectreStatus = getSpectre (ArrayReference<uint8_t> (calibrationData.data(), calibrationData.size()), &spectre);
        if (!spectreStatus)
        {
            std::stringstream ss;
            ss << "Could not create ISpectre. Code : " << static_cast<int> (spectreStatus.code()) << " Additional : " << spectreStatus.description();

            LOG (ERROR) << ss.str();
            throw RuntimeError (ss.str());
        }
        return std::unique_ptr<ISpectre> (spectre);
    }

    std::unique_ptr<ISpectre> createSpectreFakeCalib (ArrayHolder<uint8_t> &calibrationData)
    {
        ISpectre *spectre = nullptr;

        auto spectreStatus = getSpectre (calibrationData, &spectre);
        if (!spectreStatus)
        {
            std::stringstream ss;
            ss << "Could not create ISpectre. Code : " << static_cast<int> (spectreStatus.code()) << " Additional : " << spectreStatus.description();

            LOG (ERROR) << ss.str();
            throw RuntimeError (ss.str());
        }
        return std::unique_ptr<ISpectre> (spectre);
    }

    const royale::ProcessingParameterMap filterLevelFull =
    {
        { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
        { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (true) },
        { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (true) },
        { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (true) },
        { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
    };

    const royale::ProcessingParameterMap filterLevelLegacy =
    {
        { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
        { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
    };

    const royale::ProcessingParameterMap filterLevelOff =
    {
        { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseSmoothingFilter_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseHoleFilling_Bool, royale::Variant (false) },
        { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (false) },
    };

    const std::map<royale::FilterLevel, const royale::ProcessingParameterMap> filterLevelMap =
    {
        { FilterLevel::Off, filterLevelOff },
        { FilterLevel::Legacy, filterLevelLegacy },
        { FilterLevel::Full, filterLevelFull },
    };
}

ProcessingSpectre::ProcessingSpectre (IFrameCaptureReleaser *releaser)
    : Processing (releaser),
      m_currentUseCase (nullptr),
      m_isReady (false)
{
    precalcLogLUT();
    m_ampMult = static_cast<float> (m_lutSize) / m_ampMax;
}

ProcessingSpectre::~ProcessingSpectre()
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);
}

void ProcessingSpectre::precalcLogLUT()
{
    m_logLUT.clear();
    m_logLUT.resize (m_lutSize);

    for (unsigned i = 0; i < m_lutSize; ++i)
    {
        float tmp = log10f (1.0f + 9.0f * static_cast<float> (i) / static_cast<float> (m_lutSize));
        if (tmp > 1.0f)
        {
            tmp = 1.0f;
        }
        m_logLUT[i] = static_cast<uint8_t> (255.0f * tmp);
    }
}


void ProcessingSpectre::setCalibrationData (const std::vector<uint8_t> &calibrationData)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    m_calibrationData = calibrationData;

    // Delete all Spectre instances
    for (auto &instance : m_spectres)
    {
        instance.spectre.release();
        instance.used = false;
    }

    // at least create the first instance
    // (this is needed for some functions like accessing the ROI information)
    m_spectres[0].spectre = createSpectre (m_calibrationData);

    if (!m_currentUseCase)
    {
        return;
    }

    auto currentUseCaseCopy = *m_currentUseCase;

    try
    {
        activateUseCase (currentUseCaseCopy);
    }
    catch (const Exception &)
    {
        throw CalibrationDataNotFound ("Error setting calibration data for processing");
    }
}

const std::vector<uint8_t> &ProcessingSpectre::getCalibrationData() const
{
    return m_calibrationData;
}

bool ProcessingSpectre::hasCalibrationData() const
{
    return !m_calibrationData.empty();
}

void ProcessingSpectre::getProcessingParameters (ProcessingParameterMap &parameters,
        const royale::StreamId streamId)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    if (m_parameters.count (streamId) > 0)
    {
        parameters = m_parameters[streamId];
    }
    else
    {
        throw InvalidValue ("StreamID not found!");
    }
}

void ProcessingSpectre::setProcessingParameters (const ProcessingParameterMap &parameters,
        const royale::StreamId streamId)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    auto &spectreInfo = getSpectreForStream (streamId);
    auto config = spectreInfo.spectre->extendedConfiguration();

    if (applyRoyaleParameters (parameters, *config) && spectreInfo.spectre->reconfigure (*config))
    {
        m_parameters[streamId] = convertConfiguration (*config);
    }
    else
    {
        throw InvalidValue ("Could not reconfigure Spectre!");
    }
}

void ProcessingSpectre::processFrame (std::vector<ICapturedRawFrame *> &frames,
                                      std::unique_ptr<const CapturedUseCase> capturedCase,
                                      const DepthDataItem &depthData,
                                      const royale::Vector<uint32_t> &capturedTimes,
                                      std::vector<uint32_t> &newExposureTimes)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    m_timeStamp = capturedCase->getTimestamp();
    auto &spectreInfo = getSpectreForStream (depthData.streamId);

    // TODO SK: How to get size of the raw data
    auto numPixel = spectreInfo.spectre->getInputHeight() * spectreInfo.spectre->getInputWidth();

    Input<ArrayReference> input;
    input.setIntensityFrame (ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[0]]->getImageData(),
                             numPixel), capturedTimes[spectreInfo.exposureIndices[0]]);
    if (spectreInfo.frameIndices.size() > 1)
    {
        input.setModulatedFrames<ParameterKey::FREQUENCY_1> (ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[1] + 0]->getImageData(), numPixel),
                ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[1] + 1]->getImageData(), numPixel),
                ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[1] + 2]->getImageData(), numPixel),
                ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[1] + 3]->getImageData(), numPixel),
                capturedTimes[spectreInfo.exposureIndices[1]]);
    }
    if (spectreInfo.frameIndices.size() > 2)
    {
        input.setModulatedFrames<ParameterKey::FREQUENCY_2> (ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[2] + 0]->getImageData(), numPixel),
                ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[2] + 1]->getImageData(), numPixel),
                ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[2] + 2]->getImageData(), numPixel),
                ArrayReference<uint16_t> (frames[spectreInfo.frameIndices[2] + 3]->getImageData(), numPixel),
                capturedTimes[spectreInfo.exposureIndices[2]]);
    }
    input.setTemperature (capturedCase->getIlluminationTemperature());

    // Run Spectre
    auto spectreStatus = spectreInfo.spectre->processWhole (input);
    if (!spectreStatus)
    {
        throw RuntimeError ("Error running Spectre");
    }
    newExposureTimes = std::vector<uint32_t> (spectreInfo.spectre->results<ResultType::EXPOSURE_TIMES>().data(),
                       spectreInfo.spectre->results<ResultType::EXPOSURE_TIMES>().data() + spectreInfo.spectre->results<ResultType::EXPOSURE_TIMES>().size());

    // Fill the output struct
    if (m_listeners.extendedListener)
    {
        prepareIntermediateOutput (depthData.intermediateData.get(), depthData.streamId);
        prepareDepthDataOutput (depthData.depthData.get(), depthData.streamId);
    }
    else
    {
        if (m_listeners.depthDataListener)
        {
            prepareDepthDataOutput (depthData.depthData.get(), depthData.streamId);
        }
        if (m_listeners.depthImageListener)
        {
            prepareDepthImage (depthData.depthImage.get(), depthData.streamId);
        }
        if (m_listeners.sparsePointCloudListener)
        {
            prepareSparsePointCloud (depthData.sparsePointCloud.get(), depthData.streamId);
        }
        if (m_listeners.irImageListener)
        {
            prepareIRImage (depthData.irImage.get(), depthData.streamId);
        }
    }
}

void ProcessingSpectre::setUseCase (const UseCaseDefinition &useCase)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    activateUseCase (useCase);
}

void ProcessingSpectre::activateUseCase (const UseCaseDefinition &useCase)
{
    m_isReady = false;
    m_currentUseCase = std::make_shared<UseCaseDefinition> (useCase);

    Processing::setUseCase (*m_currentUseCase);

    const auto streamIds = m_currentUseCase->getStreamIds();
    if (streamIds.size() == 0u)
    {
        throw LogicError ("Use case has no streams");
    }

    // Disable all Spectre instances
    for (auto &instance : m_spectres)
    {
        instance.used = false;
    }

    ArrayHolder<uint8_t> fakeCalib;
    if (m_calibrationData.empty())
    {
        uint16_t cols, rows;
        useCase.getImage (cols, rows);

        fakeCalib = getAutoExposureOnlyCalibration (cols, rows);
    }

    for (auto i = 0u; i < streamIds.size() && i < m_spectres.size(); i++)
    {
        if (!m_spectres[i].spectre)
        {
            if (m_calibrationData.empty())
            {
                m_spectres[i].spectre = createSpectreFakeCalib (fakeCalib);
            }
            else
            {
                m_spectres[i].spectre = createSpectre (m_calibrationData);
            }
        }
        m_spectres[i].streamId = streamIds[i];
        m_spectres[i].used = true;
        prepareSpectre (*m_currentUseCase, streamIds[i], m_spectres[i]);
    }

    m_isReady = true;
}

void ProcessingSpectre::prepareSpectre (const UseCaseDefinition &useCase,
                                        const royale::StreamId streamId, SpectreInstanceInfo &info)
{
    const auto &rawFrameSets = useCase.getRawFrameSets();
    const auto &indices = useCase.getRawFrameSetIndices (streamId, 0);
    auto config = info.spectre->basicConfiguration();

    auto modGroupIdx = 0u;
    info.frameIndices.resize (1);
    info.exposureIndices.resize (1);
    for (auto j = 0u, rawFrameIdx = 0u; j < indices.size(); j++)
    {
        const auto &frameSet = rawFrameSets[indices[j]];
        if (frameSet.isGrayscale())
        {
            info.frameIndices[0] = rawFrameIdx;
            info.exposureIndices[0] = j;
        }
        else if (frameSet.isModulated() && frameSet.countRawFrames() == SPECTRE_FRAMES_PER_MOD_MEASUREMENTS
                 && modGroupIdx < SPECTRE_MAX_4_PHASE_RAW_FRAME_SETS)
        {
            auto freqKey = modGroupIdx++ ? ParameterKey::FREQUENCY_2 : ParameterKey::FREQUENCY_1;
            info.frameIndices.push_back (rawFrameIdx);
            info.exposureIndices.push_back (j);
            setParameterOrFail (*config, freqKey, frameSet.modulationFrequency);
        }

        rawFrameIdx += narrow_cast<unsigned> (frameSet.countRawFrames());
    }

    setParameterOrFail (*config, ParameterKey::NUM_FREQUENCIES, modGroupIdx);
    auto res = info.spectre->reconfigure (*config);

    if (!res)
    {
        std::stringstream sstr;
        sstr << "Could not activate basic Spectre configuration (Error: "
             << static_cast<int> (res.code()) << "; Additional: " << res.description() << ")";
        throw LogicError (sstr.str());
    }

    auto extConf (info.spectre->extendedConfiguration());
    setParameterOrFail (*extConf, ParameterKey::USE_AUTO_EXPOSURE, false);
    auto resExtReconf = info.spectre->reconfigure (*extConf);
    if (!resExtReconf)
    {
        std::stringstream sstr;
        sstr << "Could not activate extended Spectre configuration (Error: "
             << static_cast<int> (resExtReconf.code()) << "; Additional: " << resExtReconf.description() << ")";
        throw LogicError (sstr.str());
    }
}

ProcessingSpectre::SpectreInstanceInfo &ProcessingSpectre::getSpectreForStream (const StreamId &id)
{
    for (auto &instanceInfo : m_spectres)
    {
        if (instanceInfo.streamId == id && instanceInfo.used)
        {
            return instanceInfo;
        }
    }

    throw RuntimeError ("Could not find Spectre instance for stream.");
}

VerificationStatus ProcessingSpectre::verifyRawFrameSet (const RawFrameSet &rawFrameSet)
{
    if (rawFrameSet.isGrayscale() && rawFrameSet.countRawFrames() == 1)
    {
        return VerificationStatus::SUCCESS;
    }
    if (rawFrameSet.isModulated() && (rawFrameSet.countRawFrames() == SPECTRE_FRAMES_PER_MOD_MEASUREMENTS))
    {
        return VerificationStatus::SUCCESS;
    }
    return VerificationStatus::PHASE;
}

VerificationStatus ProcessingSpectre::verifyUseCase (const UseCaseDefinition &useCase)
{
    auto rawFrameSets = useCase.getRawFrameSets();

    // Check all individual rawFrameSets first.
    for (const auto &rawFrameSet : rawFrameSets)
    {
        auto status = verifyRawFrameSet (rawFrameSet);
        if (status != VerificationStatus::SUCCESS)
        {
            return status;
        }
    }

    if (useCase.getStreamIds().size() > ROYALE_USECASE_MAX_STREAMS)
    {
        LOG (ERROR) << "Processing is configured for a maximum of " << ROYALE_USECASE_MAX_STREAMS << " streams";
        return VerificationStatus::STREAM_COUNT;
    }

    return VerificationStatus::SUCCESS;
}

bool ProcessingSpectre::hasLensCenterCalibration() const
{
    if (!m_spectres[0].spectre)
    {
        throw LogicError ("No instance of Spectre");
    }

    ArrayHolder<uint16_t> center;
    return m_spectres[0].spectre->copyCalibrationField<CalibrationField::LENS_CENTER> (center);
}

void ProcessingSpectre::getLensCenterCalibration (uint16_t &centerX, uint16_t &centerY)
{
    if (!m_spectres[0].spectre)
    {
        throw LogicError ("No instance of Spectre");
    }

    ArrayHolder<uint16_t> lensCenter;
    if (m_spectres[0].spectre->copyCalibrationField<CalibrationField::LENS_CENTER> (lensCenter))
    {
        centerX = lensCenter[0];
        centerY = lensCenter[1];
    }
    else
    {
        throw RuntimeError ("The calibration does not contain a lens center calibration.");
    }
}

void ProcessingSpectre::getLensParameters (LensParameters &params)
{
    if (!m_spectres[0].spectre)
    {
        throw LogicError ("No instance of Spectre");
    }

    ArrayHolder<float> lensParameters;
    if (m_spectres[0].spectre->copyCalibrationField<CalibrationField::LENSPARAMETERS> (lensParameters))
    {
        params.focalLength.first = lensParameters[0];
        params.focalLength.second = lensParameters[1];

        params.principalPoint.first = lensParameters[2];
        params.principalPoint.second = lensParameters[3];

        params.distortionRadial.push_back (lensParameters[4]);
        params.distortionRadial.push_back (lensParameters[5]);
        params.distortionRadial.push_back (lensParameters[8]);

        params.distortionTangential.first = lensParameters[6];
        params.distortionTangential.second = lensParameters[7];
    }
    else
    {
        throw RuntimeError ("Error retrieving lens parameters");
    }
}

void ProcessingSpectre::prepareDepthDataOutput (DepthData *target, const royale::StreamId streamId)
{
    auto &spectre = *getSpectreForStream (streamId).spectre;
    auto outCoord3d = spectre.results<ResultType::COORDINATES>();
    auto outAmplitude = spectre.results<ResultType::AMPLITUDES>();
    auto outDistanceNoise = spectre.results<ResultType::DISTANCE_NOISES>();

    target->streamId = streamId;
    target->version = 3;
    target->width = narrow_cast<uint16_t> (spectre.getOutputWidth());
    target->height = narrow_cast<uint16_t> (spectre.getOutputHeight());
    target->timeStamp = m_timeStamp;

    uint32_t numPixels = target->height * target->width;
    target->points.resize (numPixels);

    size_t idx = 0;
    for (auto i = 0u; i < numPixels; ++i, idx += 4)
    {
        DepthPoint &targetPoint = target->points[i];

        targetPoint.x = outCoord3d[idx + 0];
        targetPoint.y = outCoord3d[idx + 1];
        targetPoint.z = outCoord3d[idx + 2];

        targetPoint.grayValue = (uint16_t) outAmplitude[i];
        targetPoint.noise = outDistanceNoise[i];
        auto tempDepthConfidence = static_cast<unsigned int> (outCoord3d[idx + 3] * 256);
        targetPoint.depthConfidence = static_cast<uint8_t> (tempDepthConfidence == 256 ? 255 : tempDepthConfidence);
    }
}

void ProcessingSpectre::prepareIntermediateOutput (IntermediateData *intermediateData, const royale::StreamId streamId)
{
    auto &spectreInfo = getSpectreForStream (streamId);
    auto &spectre = *spectreInfo.spectre;
    auto outFlags = spectre.results<ResultType::FLAGS>();
    auto outIntensity = spectre.results<ResultType::INTENSITIES>();
    auto outDistance = spectre.results<ResultType::DISTANCES>();
    auto outAmplitude = spectre.results<ResultType::AMPLITUDES>();

    intermediateData->streamId = streamId;
    intermediateData->version = 3;
    intermediateData->width = narrow_cast<uint16_t> (spectre.getOutputWidth());
    intermediateData->height = narrow_cast<uint16_t> (spectre.getOutputHeight());
    intermediateData->timeStamp = m_timeStamp;
    intermediateData->numFrequencies = narrow_cast<uint32_t> (spectreInfo.frameIndices.size() - 1u);
    uint32_t numPixels = intermediateData->height * intermediateData->width;

    intermediateData->points.resize (numPixels);

    for (auto i = 0u; i < numPixels; ++i)
    {
        IntermediatePoint  &targetPoint = intermediateData->points[i];

        targetPoint.intensity = outIntensity[i];
        targetPoint.distance = outDistance[i];
        targetPoint.amplitude = outAmplitude[i];
        targetPoint.flags = outFlags[i];
    }
}

const float maxConfidenceValue = 7.0f;

inline uint16_t noiseToConfidence (const float noise, const float threshold)
{
    const float scaledNoiseLevel = maxConfidenceValue * noise / threshold;
    uint16_t scaledNoiseLevelClipped = static_cast<uint16_t> (scaledNoiseLevel < 0.0f ? 0.0f
                                       : (scaledNoiseLevel > maxConfidenceValue) ? (maxConfidenceValue)
                                       : scaledNoiseLevel);
    return ( (static_cast<uint16_t> (maxConfidenceValue) + 1) - scaledNoiseLevelClipped) & 0x07;
}

void ProcessingSpectre::doPrepareDepthImage (const ArrayReference<float> &outCoord3d, const ArrayReference<float> &outNoise,
        const float noiseThreshold, const ArrayReference<uint32_t> &outFlags,
        royale::DepthImage *target, StreamId streamId)
{
    const uint16_t invalidPixel = (1 << 13);
    target->cdData.clear();
    target->cdData.reserve (outFlags.size());

    bool validImage = useValidateImageActivated (streamId);

    for (auto i = 0u; i < outFlags.size(); ++i)
    {
        if (outFlags[i] == 0 || !validImage)
        {
            target->cdData.push_back (static_cast<uint16_t> (
                                          (static_cast<uint16_t> (outCoord3d[4 * i + 2] * 1000.0f + 0.5f) & 0x1fff) // It's supposed to be a Z image, not a distance image
                                          | ( (noiseToConfidence (outNoise[i], noiseThreshold) << 13) & 0xe000)));
        }
        else
        {
            target->cdData.push_back (invalidPixel);
        }
    }
}

void ProcessingSpectre::prepareDepthImage (royale::DepthImage *target, const royale::StreamId streamId)
{
    auto &spectre = *getSpectreForStream (streamId).spectre;
    auto outCoord3d = spectre.results<ResultType::COORDINATES>();
    auto outFlags = spectre.results<ResultType::FLAGS>();
    auto outNoise = spectre.results<ResultType::DISTANCE_NOISES>();

    target->width = narrow_cast<uint16_t> (spectre.getOutputWidth());
    target->height = narrow_cast<uint16_t> (spectre.getOutputHeight());

    royale::ProcessingParameterMap params;
    this->getProcessingParameters (params, streamId);
    float noiseThreshold = params[ProcessingFlag::NoiseThreshold_Float].getFloat();

    target->streamId = streamId;
    target->timestamp = m_timeStamp.count();

    doPrepareDepthImage (outCoord3d, outNoise, noiseThreshold, outFlags, target, streamId);
}

void ProcessingSpectre::prepareSparsePointCloud (royale::SparsePointCloud *target, const royale::StreamId streamId)
{
    auto &spectre = *getSpectreForStream (streamId).spectre;
    auto outCoord3d = spectre.results<ResultType::COORDINATES>();
    auto outFlags = spectre.results<ResultType::FLAGS>();

    bool validImage = useValidateImageActivated (streamId);

    target->streamId = streamId;
    target->xyzcPoints.clear();
    target->xyzcPoints.reserve (outFlags.size() * 4);
    target->timestamp = m_timeStamp.count();

    for (auto i = 0u; i < outFlags.size(); ++i)
    {
        if (outFlags[i] == 0 || !validImage)
        {
            target->xyzcPoints.push_back (outCoord3d[4 * i + 0]);
            target->xyzcPoints.push_back (outCoord3d[4 * i + 1]);
            target->xyzcPoints.push_back (outCoord3d[4 * i + 2]);
            target->xyzcPoints.push_back (outCoord3d[4 * i + 3]);
        }
    }
    target->numPoints = static_cast<uint32_t> (target->xyzcPoints.size() / 4);
}

inline uint8_t ProcessingSpectre::amplitudeToIRValue (float a)
{
    uint32_t idx = static_cast<uint32_t> (a * m_ampMult + 0.5f);
    if (idx >= m_lutSize)
    {
        idx = m_lutSize - 1;
    }
    return m_logLUT[idx];
}

void ProcessingSpectre::doPrepareIRImage (const ArrayReference<float> &outAmplitude, royale::IRImage *target)
{
    target->data.clear();
    target->data.resize (outAmplitude.size());
    for (auto i = 0u; i < outAmplitude.size(); ++i)
    {
        target->data[i] = amplitudeToIRValue (outAmplitude[i]);
    }
}

void ProcessingSpectre::prepareIRImage (royale::IRImage *target, const royale::StreamId streamId)
{
    auto &spectre = *getSpectreForStream (streamId).spectre;
    auto outAmplitude = spectre.results<ResultType::AMPLITUDES>();
    target->width = narrow_cast<uint16_t> (spectre.getOutputWidth());
    target->height = narrow_cast<uint16_t> (spectre.getOutputHeight());

    target->streamId = streamId;
    target->timestamp = m_timeStamp.count();

    doPrepareIRImage (outAmplitude, target);
}

inline bool ProcessingSpectre::useValidateImageActivated (StreamId streamId)
{
    royale::ProcessingParameterMap params;
    this->getProcessingParameters (params, streamId);
    return params[ProcessingFlag::UseValidateImage_Bool].getBool();
}

bool ProcessingSpectre::isReadyToProcessDepthData()
{
    // Spectre is ready if we loaded appropriate calibration data
    return m_isReady;
}

royale::String ProcessingSpectre::getProcessingName ()
{
    return royale::String ("Spectre");
}

royale::String ProcessingSpectre::getProcessingVersion ()
{
    return royale::String (spectre::getVersion());
}

bool ProcessingSpectre::needsCalibrationData()
{
    return true;
}

void ProcessingSpectre::setExposureMode (ExposureMode exposureMode,
        const StreamId streamId)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    auto &spectreInfo = getSpectreForStream (streamId);
    auto config = spectreInfo.spectre->extendedConfiguration();

    ParameterVariant spectreVariant;
    switch (exposureMode)
    {
        case ExposureMode::AUTOMATIC:
            spectreVariant.set (true);
            break;
        case ExposureMode::MANUAL:
            spectreVariant.set (false);
            break;
    }

    auto spectrePar = config->getParameterByKey (ParameterKey::USE_AUTO_EXPOSURE);
    if (spectrePar.isValid())
    {
        if (! (spectrePar.setValue (spectreVariant) && config->setParameter (spectrePar)))
        {
            throw InvalidValue ("Could not create Spectre configuration!");
        }
    }

    if (spectreInfo.spectre->reconfigure (*config))
    {
        m_parameters[streamId] = convertConfiguration (*config);
    }
    else
    {
        throw InvalidValue ("Could not reconfigure Spectre!");
    }
}

royale::ExposureMode ProcessingSpectre::getExposureMode (const royale::StreamId streamId)
{
    std::lock_guard<std::recursive_mutex> lock (m_lock);

    auto &spectreInfo = getSpectreForStream (streamId);
    auto config = spectreInfo.spectre->extendedConfiguration();

    auto spectrePar = config->getParameterByKey (ParameterKey::USE_AUTO_EXPOSURE);
    auto aeVar = spectrePar.getValue();

    bool aeOn = false;
    aeVar.get<bool> (aeOn);

    if (aeOn)
    {
        return ExposureMode::AUTOMATIC;
    }
    else
    {
        return ExposureMode::MANUAL;
    }
}

void ProcessingSpectre::setFilterLevel (royale::FilterLevel level, royale::StreamId streamId)
{
    // Change the filter level based on the current use case
    auto it = filterLevelMap.find (level);
    if (it != filterLevelMap.end())
    {
        setProcessingParameters (it->second, streamId);
    }
}

royale::FilterLevel ProcessingSpectre::getFilterLevel (royale::StreamId streamId) const
{
    auto parametersForStream = m_parameters.find (streamId);

    if (parametersForStream == m_parameters.end())
    {
        throw InvalidValue ("No parameters for streamId");
    }

    // Check if the current parameters equal one of the predefined sets
    for (auto filterLevel : filterLevelMap)
    {
        bool paramsEqual = true;
        for (auto param : filterLevel.second)
        {
            auto it = parametersForStream->second.find (param.first);

            if (it != parametersForStream->second.end() &&
                    param.second != it->second)
            {
                paramsEqual = false;
                break;
            }
        }

        if (paramsEqual)
        {
            return filterLevel.first;
        }
    }

    // Apparently the current parameters are different from all predefined sets
    return FilterLevel::Custom;
}
