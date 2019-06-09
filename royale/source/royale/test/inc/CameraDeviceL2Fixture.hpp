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

class CameraDeviceL2Fixture : public CameraDeviceL1Fixture
{
protected:
    CameraDeviceL2Fixture();
    virtual ~CameraDeviceL2Fixture();

    virtual void SetUp();
    virtual void TearDown();
};

class ExtendedDataListener : public royale::IExtendedDataListener, public MonitorableListener
{
public:
    ~ExtendedDataListener()
    {
        depthDataExposureTimes.clear();
        rawDataExposureTimes.clear();
        intermediateDataExposureTimes.clear();
    }

    void onNewData (const royale::IExtendedData *data) override
    {
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
        }


        if (data->hasDepthData() &&
                data->hasIntermediateData() &&
                data->hasRawData())
        {
            depthDataExposureTimes = data->getDepthData()->exposureTimes;
            rawDataExposureTimes = data->getRawData()->exposureTimes;
            intermediateDataExposureTimes = data->getIntermediateData()->exposureTimes;
        }
        pulse ();
    }

    royale::Vector<uint32_t> depthDataExposureTimes;
    royale::Vector<uint32_t> rawDataExposureTimes;
    royale::Vector<uint32_t> intermediateDataExposureTimes;

    royale::Vector<uint16_t> phaseAngles;
    royale::Vector<uint8_t> illuminationEnabled;
};
