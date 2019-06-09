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

#include <IRImageCAPI.h>
#include <royale/IIRImageListener.hpp>

class IRImageListenerCAPI : public royale::IIRImageListener
{
public:
    explicit IRImageListenerCAPI (ROYALE_IR_IMAGE_CALLBACK cb);

    void onNewData (const royale::IRImage *data) override;

private:
    ROYALE_IR_IMAGE_CALLBACK m_externalImageCallback;
};

class IRImageCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::IRImage *data, royale_ir_image *edCAPI);
};
