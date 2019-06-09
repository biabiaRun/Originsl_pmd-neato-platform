/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <royale.hpp>
#include <limits>

using namespace royale;

/* Note: files in this directory that are named UnitTest* can run without hardware */

/**
 * Test the edge-case that someone tries to create a camera with an invalid ID,
 * AND there is no hardware connected.  The same test (but with hardware connected)
 * is run as TestCameraManager.CreateModuleWithIncorrectId.
 */
TEST (UnitTestCameraManager, CreateModuleWithIncorrectId)
{
    CameraManager manager;
    EXPECT_EQ (manager.createCamera ("invalid"), nullptr);
}

