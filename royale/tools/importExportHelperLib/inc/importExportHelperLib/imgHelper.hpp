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

#include <QtWidgets>

#include <cstddef>
#include <memory>
#include <string>
#include <iostream>

#include <royale.hpp>

namespace royale
{
    namespace importExportHelperLib
    {
        royale::CameraStatus encodeAmplPNG (const royale::DepthData *depthData, QImage &resultImg);
        royale::CameraStatus encodeIR1PNG (const royale::DepthData *depthData, QImage &resultImg);
        royale::CameraStatus encodeIR2PNG (const royale::IntermediateData *intData, QImage &resultImg);
    }
}
