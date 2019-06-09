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

#include <royale/String.hpp>
#include <royale/Pair.hpp>
#include <royale/Vector.hpp>
#include <royale/CameraManager.hpp>
#include <royale/ICameraDevice.hpp>
#include <royale/Status.hpp>
#include <gtest/gtest.h>
#include <FileSystem.hpp>

#include <royale/IDepthDataListener.hpp>
#include <royale/IIRImageListener.hpp>
#include <royale/ISparsePointCloudListener.hpp>
#include <royale/IDepthImageListener.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

/**
 * Thrown if one of the convenience methods in the CameraDeviceL*Fixture class fails.
 */
class TestFixtureException : public royale::common::Exception
{
public:
    explicit TestFixtureException (const std::string &reason, royale::CameraStatus status) :
        Exception (reason, "", status)
    {
    }
};

class CameraDeviceL1Fixture : public ::testing::Test
{
protected:
    explicit CameraDeviceL1Fixture();
    explicit CameraDeviceL1Fixture (std::unique_ptr<royale::CameraManager> manager);
    ~CameraDeviceL1Fixture() override;

    void SetUp() override;
    void TearDown() override;

    void initCamera();

    /**
     * Attempt to find a mixed-mode usecase, will change to different use cases.  Returns true if it
     * finds a mixed-mode use case, returns false if it couldn't find a mixed-mode use case.
     *
     * \throw TestFixtureException if there's an error.
     */
    bool setMixedModeUseCase();

    /**
     * Returns true if the camera->getCameraName exactly matches the argument.
     *
     * \throw TestFixtureException if there's an error while retrieving the name.
     */
    bool cameraNameIs (const royale::String &name);

    /**
    * Returns true if the camera->getCameraName starts with the argument.
    *
    * \throw TestFixtureException if there's an error while retrieving the name.
    */
    bool cameraNameStartsWith (const royale::String &name);

    std::unique_ptr<royale::CameraManager> cameraManager;
    std::unique_ptr<royale::ICameraDevice> camera;
    royale::Vector<royale::String>         expectedUseCases;
    royale::String                         cameraId;
    bool                                   validateUseCases;
};

class MonitorableListener
{
public:
    virtual ~MonitorableListener() = default;

    /**
     * Returns true as soon as this listener has received a callback.  Returns immediately if the
     * lister has been called before the call to hasBeenCalled().
     *
     * Returns false if it times out while waiting for the callback.
     */
    bool hasBeenCalled (const std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> lock (m_mutex);
        if (!m_hasBeenCalled)
        {
            m_cv.wait_for (lock, timeout,  [this] { return m_hasBeenCalled; });
        }
        return m_hasBeenCalled;
    }

    /**
     * Clears the hasBeenCalled() flag, and returns true as soon as the next callback is received.  If
     * this times out then it will return false and leave the hasBeenCalled() flag false.
     */
    bool waitForCallback (const std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> lock (m_mutex);
        m_hasBeenCalled = false;
        m_cv.wait_for (lock, timeout,  [this] { return m_hasBeenCalled; });
        return m_hasBeenCalled;
    }

    /**
     * Clears the hasBeenCalled() flag, and waits for n callbacks before returning true.
     *
     * If this times out then it will return false.  If at least one callback was received, the
     * hasBeenCalled() flag will be set, even if less than n callbacks were received.
     */
    bool waitForCallbacks (size_t n, const std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> lock (m_mutex);
        m_hasBeenCalled = false;
        m_nCallbacks = n;
        auto result = m_cv.wait_for (lock, timeout,  [this] { return m_nCallbacks == 0; });
        return result;
    }

protected:
    /**
     * Called when hasBeenCalled() should start returning true.
     */
    void pulse()
    {
        bool triggerNotification = false;
        // scope for the lock
        {
            std::lock_guard<std::mutex> lock (m_mutex);
            if (!m_hasBeenCalled)
            {
                m_hasBeenCalled = true;
                triggerNotification = true;
            }
            if (m_nCallbacks)
            {
                m_nCallbacks--;
                triggerNotification = true;
            }
        }
        if (triggerNotification)
        {
            m_cv.notify_all();
        }
    }

private:
    bool m_hasBeenCalled = false;
    size_t m_nCallbacks = 0;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

class RecordingListener : public MonitorableListener, public royale::IRecordStopListener
{
public:
    void onRecordingStopped (const uint32_t numFrames) override
    {
        frames = numFrames;
        pulse();
    }

    std::atomic<uint32_t> frames;
};

class DepthDataListener : public MonitorableListener, public royale::IDepthDataListener
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
        pulse();
    }

    std::set<royale::StreamId> m_streamIds;
    royale::Vector<uint32_t> m_expoTimes;
    int m_count;
};

class IRImageListener : public MonitorableListener, public royale::IIRImageListener
{
public:
    IRImageListener() :
        m_count (0)
    {
    }

    void onNewData (const royale::IRImage *data) override
    {
        m_count++;
        pulse();
    }

    int m_count;
};

class SparsePointCloudListener : public MonitorableListener, public royale::ISparsePointCloudListener
{
public:
    SparsePointCloudListener() :
        m_count (0)
    {
    }

    void onNewData (const royale::SparsePointCloud *data) override
    {
        m_count++;
        pulse();
    }

    int m_count;
};

class DepthImageListener : public MonitorableListener, public royale::IDepthImageListener
{
public:
    DepthImageListener() :
        m_count (0)
    {
    }

    void onNewData (const royale::DepthImage *data) override
    {
        m_count++;
        pulse();
    }

    int m_count;
};
