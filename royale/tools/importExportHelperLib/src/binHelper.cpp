/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <importExportHelperLib/binHelper.hpp>

#include <cmath>
#include <sstream>

using namespace royale;
using namespace royale::importExportHelperLib;

royale::CameraStatus royale::importExportHelperLib::encodeAmplBIN (const royale::DepthData *depthData, std::ostream &outBIN)
{
    size_t idx = 0u;
    for (size_t i = 0; i < depthData->height; ++i)
    {
        for (size_t j = 0; j < depthData->width; ++j, ++idx)
        {
            outBIN.write ( (char *) & (depthData->points.at (idx).grayValue), sizeof (uint16_t));
        }
    }

    return royale::CameraStatus::SUCCESS;
}

royale::CameraStatus royale::importExportHelperLib::encodeDepthBIN (const royale::DepthData *depthData, std::ostream &outBIN)
{
    size_t idx = 0u;
    for (size_t i = 0; i < depthData->height; ++i)
    {
        for (size_t j = 0; j < depthData->width; ++j, ++idx)
        {
            outBIN.write ( (char *) & (depthData->points.at (idx).z), sizeof (float));
        }
    }

    return royale::CameraStatus::SUCCESS;
}

royale::CameraStatus royale::importExportHelperLib::encodeDepthMMBIN (const royale::DepthData *depthData, std::ostream &outBIN)
{
    size_t idx = 0u;
    for (size_t i = 0; i < depthData->height; ++i)
    {
        for (size_t j = 0; j < depthData->width; ++j, ++idx)
        {
            uint16_t depthMM = static_cast<uint16_t> (depthData->points.at (idx).z * 1000.0f + 0.5f);
            outBIN.write ( (char *) & (depthMM), sizeof (uint16_t));
        }
    }

    return royale::CameraStatus::SUCCESS;
}

royale::CameraStatus royale::importExportHelperLib::encodeRawBIN (const royale::RawData *rawData, std::ostream &outBIN)
{
    for (size_t x = 0u; x < rawData->rawData.size(); ++x)
    {
        size_t idx = 0u;
        for (size_t i = 0; i < rawData->height; ++i)
        {
            for (size_t j = 0; j < rawData->width; ++j, ++idx)
            {
                outBIN.write ( (char *) & (rawData->rawData.at (x) [idx]), sizeof (uint16_t));
            }
        }
    }

    return royale::CameraStatus::SUCCESS;
}
