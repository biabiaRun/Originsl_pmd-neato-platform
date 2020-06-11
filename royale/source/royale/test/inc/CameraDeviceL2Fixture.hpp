/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <CameraDeviceL1Fixture.hpp>
#include <royale/RawData.hpp>

class ExtendedDataListener : public royale::IExtendedDataListener, public MonitorableListener
{
public:
    ExtendedDataListener() :
        m_count (0)
    {
    }

    ~ExtendedDataListener()
    {
        depthDataExposureTimes.clear();
        rawDataExposureTimes.clear();
        intermediateDataExposureTimes.clear();
    }

    void onNewData (const royale::IExtendedData *data) override
    {
        m_count++;
        if (data->hasRawData())
        {
            EXPECT_GT (data->getRawData()->phaseAngles.size(), 0u);
            if (!phaseAngles.empty())
            {
                EXPECT_EQ (phaseAngles.size(), data->getRawData()->phaseAngles.size());
            }
            phaseAngles = data->getRawData()->phaseAngles;
            EXPECT_GT (data->getRawData()->illuminationEnabled.size(), 0u);
            if (!illuminationEnabled.empty())
            {
                EXPECT_EQ (illuminationEnabled.size(), data->getRawData()->illuminationEnabled.size());
            }
            illuminationEnabled = data->getRawData()->illuminationEnabled;

            rawDataExposureTimes = data->getRawData()->exposureTimes;
        }

        // Check if there is any intermediate data inside
        if (data->hasIntermediateData())
        {
            auto intermediateData = data->getIntermediateData();
            uint32_t numPixels = intermediateData->width * intermediateData->height;
            EXPECT_EQ (intermediateData->points.size(), numPixels);

            bool thereIsAmplitudeData = false;
            bool thereIsDistanceData = false;
            bool thereIsFlagData = false;
            bool thereIsIntensityData = false;
            bool thereAreValidPixels = false;
            for (const auto &curPoint : intermediateData->points)
            {
                if (curPoint.amplitude != 0.0f)
                {
                    thereIsAmplitudeData = true;
                }
                if (curPoint.distance != 0.0f)
                {
                    thereIsDistanceData = true;
                }
                if (curPoint.flags != 0)
                {
                    thereIsFlagData = true;
                }
                else
                {
                    thereAreValidPixels = true;
                }
                if (curPoint.intensity != 0.0f)
                {
                    thereIsIntensityData = true;
                }
            }

            EXPECT_TRUE (thereIsFlagData);

            if (thereAreValidPixels)
            {
                EXPECT_TRUE (thereIsAmplitudeData);
                EXPECT_TRUE (thereIsDistanceData);
                EXPECT_TRUE (thereIsIntensityData);
            }

            intermediateDataExposureTimes = data->getIntermediateData()->exposureTimes;
        }


        if (data->hasDepthData())
        {
            depthDataExposureTimes = data->getDepthData()->exposureTimes;
        }
        pulse();
    }

    royale::Vector<uint32_t> depthDataExposureTimes;
    royale::Vector<uint32_t> rawDataExposureTimes;
    royale::Vector<uint32_t> intermediateDataExposureTimes;

    royale::Vector<uint16_t> phaseAngles;
    royale::Vector<uint8_t> illuminationEnabled;
    int m_count;
};

class CameraDeviceL2Fixture : public CameraDeviceL1Fixture
{
protected:
    CameraDeviceL2Fixture();
    virtual ~CameraDeviceL2Fixture();

    virtual void SetUp();
    virtual void TearDown();

    void runUseCaseAndCheckFPS (royale::String useCase)
    {
        ExtendedDataListener extendedListener;
        camera->registerDataListenerExtended (&extendedListener);

        uint32_t nrStreams;
        ASSERT_EQ (camera->getNumberOfStreams (useCase, nrStreams), royale::CameraStatus::SUCCESS);
        if (nrStreams > 1)
        {
            LOG (DEBUG) << "This test only works for non mixed mode use cases";
            return;
        }

        ASSERT_EQ (camera->setUseCase (useCase), royale::CameraStatus::SUCCESS);

        uint16_t maxFrameRate;
        ASSERT_EQ (camera->getMaxFrameRate (maxFrameRate), royale::CameraStatus::SUCCESS);
        ASSERT_EQ (camera->startCapture(), royale::CameraStatus::SUCCESS);

        // Wait until the camera is really running
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        extendedListener.m_count = 0u;

        std::this_thread::sleep_for (std::chrono::seconds (5));
        ASSERT_EQ (camera->stopCapture(), royale::CameraStatus::SUCCESS);

        int measuredFPS = extendedListener.m_count / 5;
        LOG (DEBUG) << "Testing use case : " << useCase;
        LOG (DEBUG) << "Use Case FPS : " << maxFrameRate;
        LOG (DEBUG) << "Measured FPS : " << measuredFPS;

        EXPECT_TRUE (measuredFPS <= maxFrameRate  &&
                     measuredFPS >= maxFrameRate - 2);
    }
};

