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

#include <DepthImageCAPI.h>
#include <royale/IDepthImageListener.hpp>

class DepthImageListenerCAPI : public royale::IDepthImageListener
{
public:
    explicit DepthImageListenerCAPI (ROYALE_DEPTH_IMAGE_CALLBACK cb);

    void onNewData (const royale::DepthImage *data) override;

private:
    ROYALE_DEPTH_IMAGE_CALLBACK m_externalImageCallback;

};

class DepthImageCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::DepthImage *data, royale_depth_image *diCAPI);
};
