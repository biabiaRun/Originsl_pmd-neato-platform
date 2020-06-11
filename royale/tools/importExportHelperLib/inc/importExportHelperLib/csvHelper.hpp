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

#include <cstddef>
#include <memory>
#include <string>
#include <iostream>

#include <royale.hpp>

namespace royale
{
    namespace importExportHelperLib
    {
        royale::CameraStatus encodeDepthCSV (const royale::DepthData *depthData, std::string &outCSV);
        royale::CameraStatus encodeDepthCSV (const royale::DepthData *depthData, std::ostream &outCSV);

        royale::CameraStatus encodeAmplCSV (const royale::DepthData *depthData, std::string &outCSV);
        royale::CameraStatus encodeAmplCSV (const royale::DepthData *depthData, std::ostream &outCSV);

        royale::CameraStatus encodeRawCSV (const royale::RawData *rawData, std::ostream &outCSV);
    }
}
