/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <CameraDeviceL4Fixture.hpp>

using namespace royale;

CameraDeviceL4Fixture::CameraDeviceL4Fixture() :
    CameraDeviceL1Fixture (std::unique_ptr<CameraManager>
{
    new CameraManager (ROYALE_ACCESS_CODE_LEVEL4)
})
{
}

CameraDeviceL4Fixture::~CameraDeviceL4Fixture()
{
}

void CameraDeviceL4Fixture::initCamera (const String &initUseCase)
{
    ASSERT_NE (camera, nullptr);

    auto status = camera->initialize (initUseCase);
    ASSERT_EQ (status, CameraStatus::SUCCESS);
}

void CameraDeviceL4Fixture::SetUp()
{
    CameraDeviceL1Fixture::SetUp();
}

void CameraDeviceL4Fixture::TearDown()
{
    CameraDeviceL1Fixture::TearDown();
}
