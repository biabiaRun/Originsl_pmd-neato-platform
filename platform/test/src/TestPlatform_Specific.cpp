/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

// Tests that are specific to the used module should be in this file

#include "TestPlatform.hpp"

using namespace royale;
using namespace platform;

TEST_F (CameraDeviceFixture, ValidateUseCases)
{
    initCamera();

    Vector<String> supportedUseCases;

    auto status = camera->getUseCases (supportedUseCases);
    ASSERT_EQ (status, CameraStatus::SUCCESS);

    ASSERT_EQ (supportedUseCases.size(), 5u);

    EXPECT_STREQ (supportedUseCases.at (0).c_str(), "MODE_5_25FPS");
    EXPECT_STREQ (supportedUseCases.at (1).c_str(), "MODE_5_30FPS");
    EXPECT_STREQ (supportedUseCases.at (2).c_str(), "MODE_5_45FPS");
    EXPECT_STREQ (supportedUseCases.at (3).c_str(), "MODE_5_50FPS");
    EXPECT_STREQ (supportedUseCases.at (4).c_str(), "MODE_5_60FPS");
}

TEST_F (CameraDeviceFixture, CheckFPS)
{
    camera->setCallbackData (CallbackData::Raw);

    initCamera();

#ifdef ROYALE_ENABLE_PLATFORM_CODE
    runUseCaseAndCheckFPS ("MODE_5_25FPS", 25);
    runUseCaseAndCheckFPS ("MODE_5_30FPS", 30);
    runUseCaseAndCheckFPS ("MODE_5_45FPS", 45);
    runUseCaseAndCheckFPS ("MODE_5_50FPS", 50);
    runUseCaseAndCheckFPS ("MODE_5_60FPS", 60);
#else
    runUseCaseAndCheckFPS ("Fast_Acquisition", 45);
#endif
}
