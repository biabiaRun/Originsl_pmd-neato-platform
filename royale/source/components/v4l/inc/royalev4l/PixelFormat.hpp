/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <linux/videodev2.h>
#include <inttypes.h>

#include <common/exceptions/LogicError.hpp>

namespace royale
{

    namespace v4l
    {
        enum v4l2PixelFormat
        {
            PIX_FMT_SBGGR12,
            PIX_FMT_SBGGR12P,
            PIX_FMT_RGB565,
            PIX_FMT_MAX,
        };

        inline uint32_t getV4l2Format (v4l2PixelFormat format)
        {
            switch (format)
            {
                case PIX_FMT_SBGGR12:
                    return V4L2_PIX_FMT_SBGGR12;
                case PIX_FMT_SBGGR12P:
                    return V4L2_PIX_FMT_SBGGR12P;
                case PIX_FMT_RGB565:
                    return V4L2_PIX_FMT_RGB565;
                default:
                    throw royale::common::LogicError ("Unknown v4l2 format");
            }
        }
    } /* namespace v4l */

} /* namespace royale */
