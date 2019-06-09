/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <private/SparsePointCloudListenerCAPI.hpp>

SparsePointCloudListenerCAPI::SparsePointCloudListenerCAPI (ROYALE_SPC_DATA_CALLBACK cb) :
    m_externalDataCallback (cb)
{

}

void SparsePointCloudListenerCAPI::onNewData (const royale::SparsePointCloud *data)
{
    if (m_externalDataCallback != nullptr)
    {
        royale_sparse_point_cloud spcCAPI;
        SparsePointCloudCAPIConverter::fromRoyaleData (data, &spcCAPI);
        m_externalDataCallback (&spcCAPI);
    }
}

void SparsePointCloudCAPIConverter::fromRoyaleData (
    const royale::SparsePointCloud *data, royale_sparse_point_cloud *spcCAPI)
{
    spcCAPI->timestamp = data->timestamp;
    spcCAPI->stream_id = data->streamId;
    spcCAPI->nr_points = data->numPoints;
    spcCAPI->xyzcPoints = (float *) &data->xyzcPoints[0];
}
