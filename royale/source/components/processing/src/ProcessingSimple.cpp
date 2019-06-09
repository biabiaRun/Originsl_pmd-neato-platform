/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <processing/ProcessingSimple.hpp>

#include <common/exceptions/NotImplemented.hpp>

#include <array>
#include <cstdint>
#include <vector>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

using namespace royale::processing;
using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;

namespace
{
    const float FLOAT_PI = (float) M_PI;
}

ProcessingSimple::ProcessingSimple (IFrameCaptureReleaser *releaser)
    : Processing (releaser)
{
}

ProcessingSimple::~ProcessingSimple()
{
    std::lock_guard<std::mutex> lock (m_lock);
}


void ProcessingSimple::setCalibrationData (const std::vector<uint8_t> &calibrationData)
{
    m_calibrationData = calibrationData;
}

const std::vector<uint8_t> &ProcessingSimple::getCalibrationData() const
{
    return m_calibrationData;
}

bool ProcessingSimple::hasCalibrationData() const
{
    return false;
}

void ProcessingSimple::processFrame (std::vector<ICapturedRawFrame *> &frames,
                                     std::unique_ptr<const CapturedUseCase> capturedCase,
                                     const DepthDataItem &depthData,
                                     const royale::Vector<uint32_t> &capturedTimes,
                                     std::vector<uint32_t> &newExposureTimes)
{
    std::lock_guard<std::mutex> lock (m_lock);


    // Calculate image
    depthData.depthData->timeStamp = capturedCase->getTimestamp();
    depthData.depthData->streamId = depthData.streamId;
    depthData.depthData->version = 0;
    depthData.depthData->width = m_currentWidth;
    depthData.depthData->height = m_currentHeight;
    depthData.depthData->exposureTimes = capturedTimes;
    auto block = depthData.depthData->width * depthData.depthData->height;
    depthData.depthData->points.resize (block);

    std::array<uint16_t *, 4> framePointers;
    const auto firstRawFrame = m_firstRawFrame[depthData.streamId];
    for (auto i = 0u; i < 4; ++i)
    {
        framePointers[i] = frames[firstRawFrame + i]->getImageData();
    }

    const auto range = m_range [depthData.streamId];
    for (auto i = 0; i < block; ++i)
    {
        DepthPoint &currentPoint = depthData.depthData->points[i];

        float d0, d1, d2, d3;
        d0 = (float) framePointers[0][i];
        d1 = (float) framePointers[1][i];
        d2 = (float) framePointers[2][i];
        d3 = (float) framePointers[3][i];

        float phase = FLOAT_PI + atan2f ( (d1 - d3), - (d0 - d2));
        phase *= range / (2.0f * FLOAT_PI);
        currentPoint.x = viewingVectors[0][i] * phase;
        currentPoint.y = viewingVectors[1][i] * phase;
        currentPoint.z = viewingVectors[2][i] * phase;

        float tmp1 = d3 - d1;
        tmp1 *= tmp1;
        float tmp2 = d0 - d2;
        tmp2 *= tmp2;

        currentPoint.grayValue = (uint16_t) (0.5f * sqrtf ( (float) (tmp1 + tmp2)));
        currentPoint.depthConfidence = 255;
    }
}

void ProcessingSimple::setUseCase (const UseCaseDefinition &useCase)
{
    std::lock_guard<std::mutex> lock (m_lock);
    Processing::setUseCase (useCase);
    useCase.getImage (m_currentWidth, m_currentHeight);

    const float speedOfLight = 299704644.5391500290f;

    m_firstRawFrame.clear ();
    m_range.clear ();

    for (const auto streamId : useCase.getStreamIds())
    {
        m_firstRawFrame[streamId] = 0;
        // each FrameGroup will have a similar set of frames, only calculate from the first set
        for (const auto &rawFrameSetIdx : useCase.getRawFrameSetIndices (streamId, 0))
        {
            const auto &rawFrameSet = useCase.getRawFrameSets().at (rawFrameSetIdx);
            if (rawFrameSet.isModulated() && rawFrameSet.phaseDefinition == RawFrameSet::PhaseDefinition::MODULATED_4PH_CW)
            {
                // We found the starting point
                m_range[streamId] = 0.5f * speedOfLight / static_cast<float> (rawFrameSet.modulationFrequency);
                calcViewingVectors (useCase);
                break;
            }
            else
            {
                m_firstRawFrame[streamId] += rawFrameSet.countRawFrames();
            }
        }
    }
}

VerificationStatus ProcessingSimple::verifyUseCase (const UseCaseDefinition &useCase)
{
    for (const auto &rawFrameSet : useCase.getRawFrameSets())
    {
        if (rawFrameSet.isModulated() && rawFrameSet.phaseDefinition == RawFrameSet::PhaseDefinition::MODULATED_4PH_CW)
        {
            return VerificationStatus::SUCCESS;
        }
    }

    return VerificationStatus::PHASE;
}

void ProcessingSimple::getLensCenterCalibration (uint16_t &centerX, uint16_t &centerY)
{
    throw NotImplemented ("For simple processing this is not implemented!");
}

void ProcessingSimple::getLensParameters (LensParameters &params)
{
    throw NotImplemented ("For simple processing this is not implemented!");
}

void ProcessingSimple::calcViewingVectors (const UseCaseDefinition &useCase)
{
    // This only uses a very simple viewing vector calculation

    uint16_t width, height;
    useCase.getImage (width, height);
    auto block = width * height;

    viewingVectors[0].resize (block);
    viewingVectors[1].resize (block);
    viewingVectors[2].resize (block);

    float *itx = &viewingVectors[0][0];
    float *ity = &viewingVectors[1][0];
    float *itz = &viewingVectors[2][0];

    double tz = 1.0 / tan (80.0 / 2.0); // 60 degree viewing angle
    double tx, ty; // X and Y have to be calculated for every pixel

    double aspect = 1.0;

    for (uint16_t y = 0; y < height; ++y)
    {
        for (uint16_t x = 0; x < width; ++x, ++itx, ++ity, ++itz)
        {
            tx = double ( (x) - (width - 1.0) / 2.0) / double ( (width - 1.0) / 2.0);  // X
            ty = - (1.0 / aspect) * double ( (y) - (height - 1.0) / 2.0) / double ( (height - 1.0) / 2.0);  // Y

            double norm = 1.0 / (sqrt (tx * tx + ty * ty + tz * tz));

            *itx = float (norm * tx); // X
            *ity = float (norm * -ty); // Y
            *itz = float (norm * -tz); // Z
        }
    }

}

bool ProcessingSimple::isReadyToProcessDepthData()
{
    // the simple processing doesn't need additional information
    // so it is always ready
    return true;
}

royale::String ProcessingSimple::getProcessingName ()
{
    return royale::String ("Simple");
}

royale::String ProcessingSimple::getProcessingVersion ()
{
    return royale::String ("0");
}

bool ProcessingSimple::needsCalibrationData()
{
    return false;
}

void ProcessingSimple::setExposureMode (ExposureMode exposureMode,
                                        const StreamId streamId)
{
    // For ProcessingSimple we will just silently ignore this
}

royale::ExposureMode ProcessingSimple::getExposureMode (const royale::StreamId streamId)
{
    return ExposureMode::MANUAL;
}

void ProcessingSimple::setFilterLevel (royale::FilterLevel level, royale::StreamId streamId)
{
    throw NotImplemented ("For simple processing this is not implemented!");
}

royale::FilterLevel ProcessingSimple::getFilterLevel (royale::StreamId streamId) const
{
    throw NotImplemented ("For simple processing this is not implemented!");
}
