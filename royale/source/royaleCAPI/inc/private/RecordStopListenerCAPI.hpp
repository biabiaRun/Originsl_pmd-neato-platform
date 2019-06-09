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

#include <RecordCAPI.h>
#include <royale/IRecordStopListener.hpp>

class RecordStopListenerCAPI : public royale::IRecordStopListener
{
public:
    explicit RecordStopListenerCAPI (ROYALE_RECORD_STOP_CALLBACK cb);

    void onRecordingStopped (const uint32_t numFrames) override;

private:
    ROYALE_RECORD_STOP_CALLBACK m_externalRecordStopCallback;
};
