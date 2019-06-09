/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <private/IRImageListenerCAPI.hpp>


IRImageListenerCAPI::IRImageListenerCAPI (ROYALE_IR_IMAGE_CALLBACK cb) :
    m_externalImageCallback (cb)
{

}

void IRImageListenerCAPI::onNewData (const royale::IRImage *data)
{
    if (m_externalImageCallback != nullptr)
    {
        royale_ir_image irCAPI;
        IRImageCAPIConverter::fromRoyaleData (data, &irCAPI);
        m_externalImageCallback (&irCAPI);
    }
}

void IRImageCAPIConverter::fromRoyaleData (
    const royale::IRImage *data, royale_ir_image *irCAPI)
{
    irCAPI->timestamp = data->timestamp;
    irCAPI->stream_id = data->streamId;
    irCAPI->width = data->width;
    irCAPI->height = data->height;
    irCAPI->nr_data_entries = (uint32_t) data->data.size();
    irCAPI->data = (uint8_t *) &data->data[0];
}
