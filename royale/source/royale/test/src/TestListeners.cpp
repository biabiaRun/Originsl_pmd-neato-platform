/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/CameraManager.hpp>
#include <royale/ICameraDevice.hpp>
#include <royale/Status.hpp>
#include <royale/IReplay.hpp>
#include <gtest/gtest.h>
#include <FileSystem.hpp>

#include <CameraDeviceL1Fixture.hpp>

#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>
#include <memory>

using namespace royale;
using namespace royale::common;

class TestDepthImageListener : public DepthImageListener
{
public:

    TestDepthImageListener() :
        DepthImageListener()
    {
        readFileToVector (ROYALE_TEST_FILE_PATH "/ListenerTest_Depth.dat", testData);
    }

    ~TestDepthImageListener()
    {
    }

    void onNewData (const DepthImage *data) override
    {
        receivedData = data->cdData;
        m_count++;
        pulse();
    }

    void checkData()
    {
        static const uint16_t maxDeviation = 5u;

        for (auto i = 0u; i < receivedData.size(); ++i)
        {
            uint16_t testDepth = testData[i] & 0x1fff;
            uint16_t recDepth = receivedData[i] & 0x1fff;

            EXPECT_NEAR (testDepth, recDepth, maxDeviation);
        }
    }

    royale::Vector<uint16_t> testData;
    royale::Vector<uint16_t> receivedData;
};

class TestDepthIRImageListener : public DepthIRImageListener
{
public:

    TestDepthIRImageListener() :
        DepthIRImageListener()
    {
        readFileToVector (ROYALE_TEST_FILE_PATH "/ListenerTest_Depth.dat", testDpData);
        readFileToVector (ROYALE_TEST_FILE_PATH "/ListenerTest_IR.dat", testIRData);
    }

    ~TestDepthIRImageListener()
    {
    }

    void onNewData (const DepthIRImage *data) override
    {
        receivedDpData = data->dpData;
        receivedIRData = data->irData;
        m_count++;
        pulse();
    }

    void checkData()
    {
        static const uint16_t maxDeviation = 5u;

        for (auto i = 0u; i < receivedDpData.size(); ++i)
        {
            uint16_t testDepth = testDpData[i] & 0x1fff;
            uint16_t recDepth = receivedDpData[i] & 0x1fff;

            EXPECT_NEAR (testDepth, recDepth, maxDeviation);
        }

        for (auto i = 0u; i < receivedIRData.size(); ++i)
        {
            EXPECT_NEAR (testIRData[i], receivedIRData[i], maxDeviation);
        }
    }

    royale::Vector<uint16_t> testDpData;
    royale::Vector<uint16_t> receivedDpData;

    royale::Vector<uint8_t> testIRData;
    royale::Vector<uint8_t> receivedIRData;
};

class TestIRImageListener : public IRImageListener

{
public:

    TestIRImageListener() :
        IRImageListener()
    {
        readFileToVector (ROYALE_TEST_FILE_PATH "/ListenerTest_IR.dat", testData);
    }

    ~TestIRImageListener()
    {
    }

    void onNewData (const IRImage *data) override
    {
        receivedData = data->data;
        m_count++;
        pulse();
    }

    void checkData()
    {
        static const uint8_t maxDeviation = 5u;

        for (auto i = 0u; i < receivedData.size(); ++i)
        {
            EXPECT_NEAR (testData[i], receivedData[i], maxDeviation);
        }
    }

    royale::Vector<uint8_t> testData;
    royale::Vector<uint8_t> receivedData;
};

class TestSparsePointCloudListener : public SparsePointCloudListener

{
public:

    TestSparsePointCloudListener() :
        SparsePointCloudListener()
    {
        readFileToVector (ROYALE_TEST_FILE_PATH "/ListenerTest_SPC.dat", testData);
    }

    ~TestSparsePointCloudListener()
    {
    }

    void onNewData (const SparsePointCloud *data) override
    {
        receivedData = data->xyzcPoints;
        m_count++;
        pulse();
    }

    void checkData()
    {
        static const float maxDeviationX = 0.1f;
        static const float maxDeviationY = 0.1f;
        static const float maxDeviationZ = 0.05f;
        static const float maxDeviationC = 0.001f;

        for (auto i = 0u; i < receivedData.size() / 4u; ++i)
        {
            auto j = i * 4u;

            EXPECT_NEAR (testData[j + 0u], receivedData[j + 0u], maxDeviationX);
            EXPECT_NEAR (testData[j + 1u], receivedData[j + 1u], maxDeviationY);
            EXPECT_NEAR (testData[j + 2u], receivedData[j + 2u], maxDeviationZ);
            EXPECT_NEAR (testData[j + 3u], receivedData[j + 3u], maxDeviationC);
        }
    }

    royale::Vector<float> testData;
    royale::Vector<float> receivedData;
};

TEST_F (CameraDeviceL1Fixture, TestListeners)
{
    const royale::String rrfFilename{ ROYALE_TEST_FILE_PATH "/ListenerTest.rrf" };
    ASSERT_TRUE (fileexists (rrfFilename)) << "ListenerTest.rrf not found!";

    CameraManager manager;
    auto camera = manager.createCamera (rrfFilename);
    ASSERT_NE (nullptr, camera);

    std::shared_ptr<IDepthImageListener> testDepthImageListener{ new TestDepthImageListener() };
    std::shared_ptr<IIRImageListener> testIRImageListener{ new TestIRImageListener() };
    std::shared_ptr<ISparsePointCloudListener> testSparsePointListener{ new TestSparsePointCloudListener() };
    std::shared_ptr<IDepthIRImageListener> testDepthIRImageListener{ new TestDepthIRImageListener() };

    auto status = camera->initialize();
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    status = camera->registerDepthImageListener (testDepthImageListener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);
    status = camera->registerIRImageListener (testIRImageListener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);
    status = camera->registerSparsePointCloudListener (testSparsePointListener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);
    status = camera->registerDepthIRImageListener (testDepthIRImageListener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    TestDepthImageListener *dil = static_cast<TestDepthImageListener *> (testDepthImageListener.get());
    TestIRImageListener *iil = static_cast<TestIRImageListener *> (testIRImageListener.get());
    TestSparsePointCloudListener *spl = static_cast<TestSparsePointCloudListener *> (testSparsePointListener.get());
    TestDepthIRImageListener *diil = static_cast<TestDepthIRImageListener *> (testDepthIRImageListener.get());

    EXPECT_EQ (CameraStatus::SUCCESS, camera->startCapture());

    EXPECT_TRUE (spl->hasBeenCalled (std::chrono::seconds (1)));
    EXPECT_TRUE (spl->waitForCallback (std::chrono::seconds (1)));
    EXPECT_TRUE (dil->hasBeenCalled (std::chrono::seconds (1)));
    EXPECT_TRUE (iil->hasBeenCalled (std::chrono::seconds (1)));
    EXPECT_TRUE (diil->hasBeenCalled (std::chrono::seconds (1)));

    EXPECT_EQ (CameraStatus::SUCCESS, camera->stopCapture());

    EXPECT_EQ (spl->receivedData.size(), 148000u);

    dil->checkData();
    iil->checkData();
    spl->checkData();
    diil->checkData();
}
