#include <royale/CameraManager.hpp>
#include <royale/ICameraDevice.hpp>
#include <royale/Status.hpp>
#include <gtest/gtest.h>

#include <thread>
#include <chrono>

using namespace royale;

TEST (TestCameraManager, GetConnectedCameraList)
{
    std::unique_ptr<ICameraDevice> camera;
    {
        CameraManager manager;
#if defined(TARGET_PLATFORM_ANDROID)
        auto connectedCameras = manager.getConnectedCameraList (0);
#else
        auto connectedCameras = manager.getConnectedCameraList();
#endif
        EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

        ASSERT_FALSE (connectedCameras.empty());
        camera = manager.createCamera (connectedCameras[0]);
    }

    // guarantee that the camera is valid after the CameraManager scope
    EXPECT_NE (camera, nullptr);
}

/**
 * Test the edge-case that someone tries to create a camera with an invalid ID,
 * AND there is hardware connected.  The same test (but without hardware connected)
 * is run as UnitTestCameraManager.CreateModuleWithIncorrectId.
 */
TEST (TestCameraManager, CreateModuleWithIncorrectId)
{
    CameraManager manager;
    EXPECT_EQ (manager.createCamera ("invalid"), nullptr);
}

TEST (TestCameraManager, TestLevel1)
{
    CameraManager manager;
    std::unique_ptr<ICameraDevice> camera;

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = manager.getConnectedCameraList (0);
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

    ASSERT_FALSE (connectedCameras.empty());
    camera = manager.createCamera (connectedCameras[0]);
    CameraAccessLevel accessLevel;
    EXPECT_EQ (CameraStatus::SUCCESS, camera->getAccessLevel (accessLevel));
    EXPECT_EQ (CameraAccessLevel::L1, accessLevel);
}

TEST (TestCameraManager, TestLevel2)
{
    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL2);
    std::unique_ptr<ICameraDevice> camera;

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = manager.getConnectedCameraList (0);
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

    ASSERT_FALSE (connectedCameras.empty());
    camera = manager.createCamera (connectedCameras[0]);
    CameraAccessLevel accessLevel;
    EXPECT_EQ (CameraStatus::SUCCESS, camera->getAccessLevel (accessLevel));
    EXPECT_EQ (CameraAccessLevel::L2, accessLevel);
}

TEST (TestCameraManager, TestLevel3)
{
    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL3);
    std::unique_ptr<ICameraDevice> camera;

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = manager.getConnectedCameraList (0);
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

    ASSERT_FALSE (connectedCameras.empty());
    camera = manager.createCamera (connectedCameras[0]);
    CameraAccessLevel accessLevel;
    EXPECT_EQ (CameraStatus::SUCCESS, camera->getAccessLevel (accessLevel));
    EXPECT_EQ (CameraAccessLevel::L3, accessLevel);
}

TEST (TestCameraManager, TestLevel4)
{
    CameraManager manager (ROYALE_ACCESS_CODE_LEVEL4);
    std::unique_ptr<ICameraDevice> camera;

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = manager.getConnectedCameraList (0);
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

    ASSERT_FALSE (connectedCameras.empty());
    camera = manager.createCamera (connectedCameras[0]);
    CameraAccessLevel accessLevel;
    EXPECT_EQ (CameraStatus::SUCCESS, camera->getAccessLevel (accessLevel));
    EXPECT_EQ (CameraAccessLevel::L4, accessLevel);
}

TEST (TestCameraManager, TestGetConnectedCameraListTwice)
{
    CameraManager manager;

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = manager.getConnectedCameraList (0);
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras2 = manager.getConnectedCameraList (0);
#else
    auto connectedCameras2 = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras2.size()), 1u);
}

TEST (TestCameraManager, TestTwoInstancesOfCameraMangerAftwards)
{
    {
        CameraManager m;
        auto l1 = m.getConnectedCameraList();

        ASSERT_EQ (l1.size(), 1u);
    }
    {
        CameraManager m;
        auto l1 = m.getConnectedCameraList();

        ASSERT_EQ (l1.size(), 1u);
    }
}

TEST (TestCameraManager, TestGetConnectedCameraListTwiceConsumeOne)
{
    CameraManager manager;

#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = manager.getConnectedCameraList (0);
#else
    auto connectedCameras = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);
    ASSERT_FALSE (connectedCameras.empty());
    auto camera = manager.createCamera (connectedCameras[0]);

    ASSERT_NE (camera, nullptr);

    auto status = camera->initialize();
    ASSERT_EQ (status, CameraStatus::SUCCESS);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    std::this_thread::sleep_for (std::chrono::seconds (2));

    // if one camera is plugged in no other should be detected
#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras2 = manager.getConnectedCameraList (0);
#else
    auto connectedCameras2 = manager.getConnectedCameraList();
#endif
    EXPECT_EQ (static_cast<unsigned int> (connectedCameras2.size()), 0u);

    EXPECT_NO_THROW (EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS));
}
