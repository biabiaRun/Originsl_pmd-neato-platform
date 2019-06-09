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
#include <gtest/gtest.h>
#include <FileSystem.hpp>

#include <CameraDeviceL4Fixture.hpp>

#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>
#include <memory>

using namespace royale;

namespace
{
    //MODE_5_45FPS_1000 for Evalboard with MiraCE A12
    const Vector<Pair<String, uint64_t>> manualConfig_EvalboardMiraCEA12 =
    {
        { "0xa893", 0x2241 },
        { "0xa894", 0x13b2 },
        { "0xa895", 0x03bb },
        { "0xa896", 0x2041 },
        { "0xa897", 0x13b1 },
        { "0xa898", 0x03bb },
        { "0xa899", 0x4459 },
        { "0xa89a", 0xc4ec },
        { "0xa89b", 0x0bce },
        { "0x9813", 0x13ff },

        { "0xa882", 0x0280 },
        { "0xa883", 0x1057 },
        { "0xa884", 0x0000 },

        { "0xa88a", 0x0006 },
        { "0xa88b", 0x0060 },
        { "0xa88e", 0x0380 },
        { "0xa88f", 0x000f },
        { "0xa890", 0x1e0a },
        { "0xa891", 0x1c05 },
        { "0xa892", 0x0011 },
        { "0xb008", 0x0180 },
        { "0xb009", 0x3140 },
        { "0xb00a", 0x018d },

        { "0xb00e", 0x1515 },
        { "0xb00f", 0x1515 },
        { "0xb010", 0x1515 },
        { "0xb011", 0x1515 },
        { "0xb012", 0x1515 },
        { "0xb013", 0x1515 },
        { "0xb014", 0x1515 },
        { "0xb015", 0x1515 },
        { "0xb016", 0x1515 },
        { "0xb017", 0x0415 },
        { "0xb018", 0x0404 },
        { "0xb019", 0x0004 },
        { "0xb046", 0x0004 },
        { "0xb047", 0x0000 },

        { "0xb061", 0x0400 },
        { "0xb06e", 0x2124 },
        { "0xb086", 0x1513 },
        { "0xa88c", 0x0410 },
        { "0xc401", 0x0001 },
        { "0xc400", 0x0001 },
        { "0x9802", 0x0000 },

        { "0xa800", 0x0ea6 },
        { "0xa801", 0x0000 },
        { "0xa802", 0x0004 },
        { "0xa803", 0x0000 },
        { "0xa804", 0x0ea6 },
        { "0xa805", 0x0000 },
        { "0xa806", 0x0048 },
        { "0xa807", 0x0000 },
        { "0xa808", 0x0ea6 },
        { "0xa809", 0x0000 },
        { "0xa80a", 0x008c },
        { "0xa80b", 0x0000 },
        { "0xa80c", 0x0ea6 },
        { "0xa80d", 0x0000 },
        { "0xa80e", 0x00c0 },
        { "0xa80f", 0x0000 },
        { "0xa810", 0x0ea6 },
        { "0xa811", 0x03b3 },
        { "0xa812", 0x2000 },
        { "0xa813", 0x0000 },
        { "0xa885", 0x0004 },
        { "0xa886", 0x0000 },
        { "0xa887", 0x015f },
        { "0xa888", 0x0001 },
        { "0xa889", 0x0120 },
        { "0xa88d", 0x0004 },
        { "0xa888", 0x0000 },
        { "0xa889", 0x011f },
    };
}

TEST_F (CameraDeviceL4Fixture, ManualStartup_ForEvalboard)
{
    //this integration test verifies if the L3 function to have full control
    //over the imager hardware is able to configure the imager hardware
    //in a way so royale is able to receive image data
    if (!cameraNameStartsWith ("EVALBOARD_"))
    {
        // This test is for the evalboard only as we have provided a configuration for it
        LOG (WARN) << "Please connect a MiraCE A12 Evalboard";
        return;
    }

    //initialize the system to be able to receive image data according to the specified use case
    initCamera ("MODE_5_45FPS_1000");

    //provide the manual configuration (ideally matches the configured use case)
    ASSERT_EQ (camera->writeRegisters (manualConfig_EvalboardMiraCEA12), CameraStatus::SUCCESS);

    //register data callback to be able to receive incoming image data
    std::shared_ptr<IExtendedDataListener> listener{ new ExtendedDataListener() };
    camera->registerDataListenerExtended (listener.get());

    //set the imager start trigger prior to start image capturing by royale
    camera->writeRegisters ({ { "43136", 1 } });
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    //wait some time to be able to receive image data
    std::this_thread::sleep_for (std::chrono::seconds (2));

    //stop image capturing by royale before switching off the imager itself
    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
    camera->writeRegisters ({ { "43136", 0 } });

    //some frames should have been received
    ExtendedDataListener *elistener = reinterpret_cast<ExtendedDataListener *> (listener.get());
    EXPECT_GT (elistener->depthDataExposureTimes.size(), 0u);
}
