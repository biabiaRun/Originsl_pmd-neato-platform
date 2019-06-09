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

#include <DepthDataCAPI.h>
#include <royale/IDepthDataListener.hpp>

class DepthDataListenerCAPI : public royale::IDepthDataListener
{
public:
    explicit DepthDataListenerCAPI (ROYALE_DEPTH_DATA_CALLBACK cb);

    void onNewData (const royale::DepthData *data) override;

private:
    ROYALE_DEPTH_DATA_CALLBACK m_externalDataCallback;
};

class DepthDataCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::DepthData *data, royale_depth_data *ddCAPI);
};
