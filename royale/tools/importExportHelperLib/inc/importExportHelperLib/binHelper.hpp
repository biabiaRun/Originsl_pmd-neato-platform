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

#include <string>
#include <iostream>

#include <royale.hpp>

namespace royale
{
    namespace importExportHelperLib
    {
        royale::CameraStatus encodeAmplBIN (const royale::DepthData *depthData, std::ostream &outBIN);
        royale::CameraStatus encodeDepthBIN (const royale::DepthData *depthData, std::ostream &outBIN);
        royale::CameraStatus encodeDepthMMBIN (const royale::DepthData *depthData, std::ostream &outBIN);
        royale::CameraStatus encodeRawBIN (const royale::RawData *rawData, std::ostream &outBIN);
    }
}
