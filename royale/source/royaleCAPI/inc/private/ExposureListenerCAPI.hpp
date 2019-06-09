/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <ExposureCAPI.h>
#include <royale/IExposureListener2.hpp>

// Note: both ExposureListenerCAPI and ExposureListener2CAPI are implemented using IEL2.
class ExposureListenerCAPI : public royale::IExposureListener2
{
public:
    explicit ExposureListenerCAPI (ROYALE_EXPOSURE_CALLBACK_v210 cb);

    void onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId) override;

private:
    ROYALE_EXPOSURE_CALLBACK_v210 m_externalExposureCallback;
};

class ExposureListener2CAPI : public royale::IExposureListener2
{
public:
    explicit ExposureListener2CAPI (ROYALE_EXPOSURE_CALLBACK_v300 cb);

    void onNewExposure (const uint32_t exposureTime, const royale::StreamId streamId) override;

private:
    ROYALE_EXPOSURE_CALLBACK_v300 m_externalExposureCallback;
};
