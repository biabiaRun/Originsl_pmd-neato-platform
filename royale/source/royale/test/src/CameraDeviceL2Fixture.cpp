/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <CameraDeviceL2Fixture.hpp>

using namespace royale;

CameraDeviceL2Fixture::CameraDeviceL2Fixture() :
    CameraDeviceL1Fixture (std::unique_ptr<CameraManager>
{
    new CameraManager (ROYALE_ACCESS_CODE_LEVEL2)
})
{
}

CameraDeviceL2Fixture::~CameraDeviceL2Fixture()
{
}

void CameraDeviceL2Fixture::SetUp()
{
    CameraDeviceL1Fixture::SetUp();
}

void CameraDeviceL2Fixture::TearDown()
{
    CameraDeviceL1Fixture::TearDown();
}
