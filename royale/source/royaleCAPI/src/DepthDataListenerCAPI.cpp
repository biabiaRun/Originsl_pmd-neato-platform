/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <private/DepthDataListenerCAPI.hpp>


DepthDataListenerCAPI::DepthDataListenerCAPI (ROYALE_DEPTH_DATA_CALLBACK cb) :
    m_externalDataCallback (cb)
{

}

void DepthDataListenerCAPI::onNewData (const royale::DepthData *data)
{
    if (m_externalDataCallback != nullptr)
    {
        royale_depth_data ddCAPI;
        DepthDataCAPIConverter::fromRoyaleData (data, &ddCAPI);
        m_externalDataCallback (&ddCAPI);
    }
}

void DepthDataCAPIConverter::fromRoyaleData (
    const royale::DepthData *data, royale_depth_data *ddCAPI)
{
    ddCAPI->version = data->version;
    ddCAPI->timestamp = data->timeStamp.count();
    ddCAPI->stream_id = data->streamId;
    ddCAPI->width = data->width;
    ddCAPI->height = data->height;
    ddCAPI->nr_exposure_times = (uint32_t) data->exposureTimes.size();
    ddCAPI->exposure_times = (uint32_t *) &data->exposureTimes[0];
    ddCAPI->nr_points = (uint32_t) data->points.size();
    ddCAPI->points = (royale_depth_point *) &data->points[0];

}
