/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <private/RecordStopListenerCAPI.hpp>

RecordStopListenerCAPI::RecordStopListenerCAPI (ROYALE_RECORD_STOP_CALLBACK cb) :
    m_externalRecordStopCallback (cb)
{

}

void RecordStopListenerCAPI::onRecordingStopped (uint32_t numFrames)
{
    if (m_externalRecordStopCallback != nullptr)
    {
        m_externalRecordStopCallback (numFrames);
    }
}
