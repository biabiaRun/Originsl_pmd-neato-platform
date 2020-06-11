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

        royale::CameraStatus encodePLY (const royale::DepthData *depthData, std::string &outPLY);

        royale::CameraStatus encodePLY (const royale::DepthData *depthData, std::ostream &outPLY);
    }
}
