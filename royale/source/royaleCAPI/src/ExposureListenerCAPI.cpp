/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <private/ExposureListenerCAPI.hpp>

ExposureListenerCAPI::ExposureListenerCAPI (ROYALE_EXPOSURE_CALLBACK_v210 cb) :
    m_externalExposureCallback (cb)
{

}

void ExposureListenerCAPI::onNewExposure (uint32_t exposureTime, const royale::StreamId streamId)
{
    if (m_externalExposureCallback != nullptr)
    {
        m_externalExposureCallback (exposureTime);
    }
}

ExposureListener2CAPI::ExposureListener2CAPI (ROYALE_EXPOSURE_CALLBACK_v300 cb) :
    m_externalExposureCallback (cb)
{

}

void ExposureListener2CAPI::onNewExposure (uint32_t exposureTime, const royale::StreamId streamId)
{
    if (m_externalExposureCallback != nullptr)
    {
        m_externalExposureCallback (exposureTime, streamId);
    }
}
