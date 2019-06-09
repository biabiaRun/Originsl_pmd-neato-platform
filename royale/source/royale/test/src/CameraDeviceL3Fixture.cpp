/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <CameraDeviceL3Fixture.hpp>

using namespace royale;

CameraDeviceL3Fixture::CameraDeviceL3Fixture() :
    CameraDeviceL1Fixture (std::unique_ptr<CameraManager>
{
    new CameraManager (ROYALE_ACCESS_CODE_LEVEL3)
})
{
}

CameraDeviceL3Fixture::~CameraDeviceL3Fixture()
{
}

void CameraDeviceL3Fixture::SetUp()
{
    CameraDeviceL1Fixture::SetUp();
}

void CameraDeviceL3Fixture::TearDown()
{
    CameraDeviceL1Fixture::TearDown();
}
