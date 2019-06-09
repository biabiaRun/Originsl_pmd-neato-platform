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

#include <CameraDeviceL1Fixture.hpp>
#include <CameraDeviceL2Fixture.hpp>
#include <royale/RawData.hpp>
#include <royale/Vector.hpp>

class CameraDeviceL3Fixture : public CameraDeviceL1Fixture
{
protected:
    CameraDeviceL3Fixture();
    virtual ~CameraDeviceL3Fixture();

    virtual void SetUp();
    virtual void TearDown();
};
