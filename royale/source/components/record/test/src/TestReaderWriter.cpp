/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

#include <record/FileReaderDispatcher.hpp>
#include <record/FileWriter.hpp>
#include <record/CameraRecord.hpp>
#include <collector/CapturedUseCase.hpp>

#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>

#include <royale/ProcessingFlag.hpp>

#include <MsvcMacros.hpp>

#include "CommonTestRecord.hpp"

using namespace std;
using namespace royale;
using namespace royale::record;
using namespace royale::common;
using namespace royale::config;
using namespace royale::usecase;
using namespace royale::collector;

namespace
{
    static const std::string testfilename = "testfile.rrf";

    /**
     * Value of CapturedUseCase.getExposureTime()[frameset] for depth frame number curFrame.
     */
    uint32_t expectedCapturedExposureTime (size_t curFrame, size_t frameSet)
    {
        return narrow_cast<uint32_t> (100 + curFrame + frameSet);
    }
}

void writeFile (std::vector<UseCaseDefinition> &ucVec)
{
    CameraRecord writer (0, 0, 0, "testCamera", static_cast<ImagerType> (0));

    std::vector<uint8_t> calibrationData;
    calibrationData.push_back (1);
    calibrationData.push_back (2);
    calibrationData.push_back (3);
    calibrationData.push_back (4);

    ProcessingParameterMap paramMap;

    paramMap[ProcessingFlag::AdaptiveNoiseFilterType_Int] = Variant (1);
    paramMap[ProcessingFlag::FlyingPixelsF0_Float] = Variant (2.345f);
    paramMap[ProcessingFlag::UseAdaptiveNoiseFilter_Bool] = Variant (true);
    paramMap[ProcessingFlag::UseValidateImage_Bool] = Variant (false);

    writer.startRecord (testfilename, calibrationData, "1234");

    for (size_t curFrame = 0; curFrame < ucVec.size(); ++curFrame)
    {
        UseCaseDefinition &uc = ucVec.at (curFrame);
        std::vector<ICapturedRawFrame *> frames;

        writer.setProcessingParameters (ProcessingParameterVector::fromStdMap (paramMap), uc.getStreamIds().at (0));

        CommonTestRecord::createFrames (frames, uc, narrow_cast<uint16_t> (curFrame));

        float illuTemp = 30.0f + static_cast<float> (curFrame);
        std::chrono::milliseconds timestamp = std::chrono::milliseconds (curFrame * 500);
        std::vector<uint32_t> capturedExposureTimes;
        for (size_t i = 0; i < uc.getExposureGroups().size(); i++)
        {
            capturedExposureTimes.push_back (expectedCapturedExposureTime (curFrame, i));
        }

        std::unique_ptr<CapturedUseCase> cuc{ new CapturedUseCase{ nullptr, illuTemp, timestamp, capturedExposureTimes } };

        writer.captureCallback (frames, uc, uc.getStreamIds().at (0), std::move (cuc));

        for (size_t i = 0; i < frames.size(); ++i)
        {
            ICapturedRawFrame *frame = frames.at (i);
            delete frame;
        }
        frames.clear();
    }

    writer.stopRecord();
}

/**
 * Load the data from the test recording, and compare it to the data written.
 *
 * @param ucVec the data that was originally written
 */
void readFile (const std::vector<UseCaseDefinition> &ucVec)
{
    FileReaderDispatcher reader;

    EXPECT_NO_THROW (reader.open (testfilename));

    EXPECT_EQ (ucVec.size(), reader.numFrames());

    EXPECT_EQ (0, memcmp (reader.imagerSerial().c_str(), "1234", 4));

    EXPECT_STREQ ("testCamera", reader.cameraName().c_str ());

    for (size_t curFrame = 0; curFrame < ucVec.size(); ++curFrame)
    {
        const UseCaseDefinition &uc = ucVec.at (curFrame);

        std::vector<std::vector<uint16_t>> frames;

        EXPECT_NO_THROW (reader.seek (narrow_cast<uint16_t> (curFrame)));

        EXPECT_EQ (curFrame, reader.currentFrame());

        ProcessingParameterMap paramMap;

        std::vector<std::vector<uint16_t>> imageData;
        std::vector<std::vector<uint16_t>> pseudoData;
        royale_frameheader_v3 frameHeader;
        std::vector<royale_streamheader_v3> streamHeaders;
        std::vector<royale_framegroupheader_v3> frameGroupHeaders;
        std::vector<royale_exposuregroupheader_v3> exposureGroupHeaders;
        std::vector<royale_rawframesetheader_v3> rawFrameSetHeaders;
        std::vector<royale_processingparameter_v3> processingParameters;
        std::vector<std::pair<std::string, std::vector<uint8_t>>> additionalData;

        EXPECT_NO_THROW (reader.get (imageData, pseudoData, &frameHeader, streamHeaders, frameGroupHeaders, exposureGroupHeaders,
                                     rawFrameSetHeaders, processingParameters, additionalData));

        ProcessingParameterMap expectedParamMap;
        expectedParamMap[ProcessingFlag::AdaptiveNoiseFilterType_Int] = Variant (1);
        expectedParamMap[ProcessingFlag::FlyingPixelsF0_Float] = Variant (2.345f);
        expectedParamMap[ProcessingFlag::UseAdaptiveNoiseFilter_Bool] = Variant (true);
        expectedParamMap[ProcessingFlag::UseValidateImage_Bool] = Variant (false);

        EXPECT_EQ (processingParameters.size(), expectedParamMap.size());
        for (auto curParam = processingParameters.begin(); curParam != processingParameters.end(); ++curParam)
        {
            EXPECT_EQ (static_cast<uint8_t> (expectedParamMap[static_cast<ProcessingFlag> (curParam->processingFlag)].variantType()), curParam->dataType);
            EXPECT_EQ (expectedParamMap[static_cast<ProcessingFlag> (curParam->processingFlag)].getData(), curParam->value);
        }

        // \todo ROYAL-3267 Should this be using the UseCase name instead of something from UseCaseDefinition?
        EXPECT_EQ (0, uc.getTypeName().substr (0, 34).compare (frameHeader.useCaseName));

        EXPECT_EQ (uc.getTargetRate(), frameHeader.targetFrameRate);

        EXPECT_EQ (uc.getRawFrameCount(), frameHeader.numRawFrames);

        const std::vector<RawFrameSet> ucRFS = uc.getRawFrameSets().toStdVector();

        EXPECT_EQ (ucRFS.size(), rawFrameSetHeaders.size());

        for (size_t i = 0; i < ucRFS.size(); ++i)
        {
            EXPECT_EQ (ucRFS[i].modulationFrequency, rawFrameSetHeaders[i].modFreq);
            EXPECT_EQ (static_cast<uint8_t> (ucRFS[i].phaseDefinition), rawFrameSetHeaders[i].phaseDefinition);

            EXPECT_EQ (expectedCapturedExposureTime (curFrame, i), rawFrameSetHeaders[i].capturedExpTime);

            EXPECT_EQ (ucRFS[i].countRawFrames(), rawFrameSetHeaders[i].numRawFrames);
        }

        uint16_t numColsExpected, numRowsExpected;
        uc.getImage (numColsExpected, numRowsExpected);

        EXPECT_EQ (numColsExpected, frameHeader.numColumns);
        EXPECT_EQ (numRowsExpected, frameHeader.numRows);

        EXPECT_EQ (static_cast<unsigned long> (curFrame * 500), static_cast<unsigned long> (frameHeader.timestamp));
        EXPECT_FLOAT_EQ (30.0f + static_cast<float> (curFrame), frameHeader.illuTemperature);

        for (size_t rawFrameIndex = 0; rawFrameIndex < frames.size(); ++rawFrameIndex)
        {
            std::vector<uint16_t> &rawDataFrame = frames.at (rawFrameIndex);
            uint16_t *data = &rawDataFrame[0];
            for (auto idx = 0; idx < frameHeader.numColumns * frameHeader.numRows; ++idx)
            {
                EXPECT_EQ (CommonTestRecord::expectedValue (narrow_cast<uint16_t> (idx), narrow_cast<uint16_t> (curFrame), narrow_cast<uint16_t> (rawFrameIndex)), data[idx]);
            }
        }

        EXPECT_EQ (0u, additionalData.size());
    }

    EXPECT_NO_THROW (reader.close());
}

TEST (TestReaderWriter, ReadWriteUC4)
{
    remove (testfilename.c_str());

    UseCaseFourPhase testUC (45u, 30000000, {50u, 1000u}, 1000u, 1000u);

    std::vector<UseCaseDefinition> ucVec;
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);

    writeFile (ucVec);
    readFile (ucVec);

    remove (testfilename.c_str ());
}

TEST (TestReaderWriter, ReadWriteUC8)
{
    remove (testfilename.c_str());

    UseCaseEightPhase testUC (10u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u);

    std::vector<UseCaseDefinition> ucVec;
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);

    writeFile (ucVec);
    readFile (ucVec);

    remove (testfilename.c_str());
}

TEST (TestReaderWriter, ReadWriteUC9)
{
    remove (testfilename.c_str());

    UseCaseEightPhase testUC (5u, 30000000, 20200000, std::pair<uint32_t, uint32_t> (50u, 1000u), 1000u, 1000u, 200u);

    std::vector<UseCaseDefinition> ucVec;
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC);

    writeFile (ucVec);
    readFile (ucVec);

    remove (testfilename.c_str());
}

TEST (TestReaderWriter, ReadWriteMixed)
{
    remove (testfilename.c_str());

    UseCaseFourPhase testUC (45u, 30000000, {50u, 1000u}, 1000u, 1000u);
    UseCaseEightPhase testUC2 (10u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u);

    std::vector<UseCaseDefinition> ucVec;
    ucVec.push_back (testUC);
    ucVec.push_back (testUC2);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC2);
    ucVec.push_back (testUC);
    ucVec.push_back (testUC2);
    ucVec.push_back (testUC2);

    writeFile (ucVec);
    readFile (ucVec);

    remove (testfilename.c_str());
}
