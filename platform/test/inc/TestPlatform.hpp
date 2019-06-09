/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <gtest/gtest.h>

#include <thread>

#include <royale/ICameraDevice.hpp>

#include <CameraFactory.hpp>

class TestFixtureException : public royale::common::Exception
{
public:
    explicit TestFixtureException (const std::string &reason, royale::CameraStatus status) :
        Exception (reason, "", status)
    {
    }
};

class DepthDataListener : public royale::IDepthDataListener
{
public:
    DepthDataListener() :
        m_count (0)
    {
    }

    void onNewData (const royale::DepthData *data) override
    {
        m_count++;
        m_streamIds.insert (data->streamId);
        m_expoTimes = data->exposureTimes;
    }

    std::set<royale::StreamId> m_streamIds;
    royale::Vector<uint32_t> m_expoTimes;
    int m_count;
};

class ExtendedDataListener : public royale::IExtendedDataListener
{
public:
    ExtendedDataListener() :
        m_count (0)
    {
    }

    void onNewData (const royale::IExtendedData *data) override
    {
        m_count++;

        if (data->hasRawData())
        {
            auto rawData = data->getRawData();
            m_temperature = rawData->illuminationTemperature;
        }
    }

    int m_count;
    float m_temperature;
};

class CameraDeviceFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        platform::CameraFactory factory;
        camera = factory.createCamera();

    }

    void TearDown() override { }

    void initCamera()
    {
        auto status = camera->initialize();
        if (status != royale::CameraStatus::SUCCESS)
        {
            throw TestFixtureException ("initCamera failed", status);
        }
    }

    void switchToFastUseCasePicoFlexx()
    {
        royale::String camName;
        camera->getCameraName (camName);

        if (camName == "PICOFLEXX")
        {
            camera->setUseCase ("Fast_Acquisition");
        }
    }

    void runUseCaseAndCheckFPS (royale::String useCase, int fps)
    {
        ExtendedDataListener extendedListener;
        camera->registerDataListenerExtended (&extendedListener);

        ASSERT_EQ (camera->setUseCase (useCase), royale::CameraStatus::SUCCESS);
        ASSERT_EQ (camera->startCapture(), royale::CameraStatus::SUCCESS);

        // Wait until the camera is really running
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        extendedListener.m_count = 0u;

        std::this_thread::sleep_for (std::chrono::seconds (5));
        ASSERT_EQ (camera->stopCapture(), royale::CameraStatus::SUCCESS);

        int measuredFPS = extendedListener.m_count / 5;
        LOG (DEBUG) << "Use Case FPS : " << fps;
        LOG (DEBUG) << "Measured FPS : " << measuredFPS;

        EXPECT_TRUE (measuredFPS >= fps - 1 &&
                     measuredFPS <= fps + 1);
    }


    std::unique_ptr<royale::ICameraDevice> camera;
};
