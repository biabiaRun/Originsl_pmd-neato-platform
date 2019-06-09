/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <private/DepthImageListenerCAPI.hpp>


DepthImageListenerCAPI::DepthImageListenerCAPI (ROYALE_DEPTH_IMAGE_CALLBACK cb) :
    m_externalImageCallback (cb)
{

}

void DepthImageListenerCAPI::onNewData (const royale::DepthImage *data)
{
    if (m_externalImageCallback != nullptr)
    {
        royale_depth_image diCAPI;
        DepthImageCAPIConverter::fromRoyaleData (data, &diCAPI);
        m_externalImageCallback (&diCAPI);
    }
}

void DepthImageCAPIConverter::fromRoyaleData (
    const royale::DepthImage *data, royale_depth_image *diCAPI)
{
    diCAPI->timestamp = data->timestamp;
    diCAPI->stream_id = data->streamId;
    diCAPI->width = data->width;
    diCAPI->height = data->height;
    diCAPI->nr_data_entries = (uint32_t) data->cdData.size();
    diCAPI->cdData = (uint8_t *) &data->cdData[0];
}
