/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <importExportHelperLib/csvHelper.hpp>

#include <common/StringFunctions.hpp>

#include <sstream>
#include <iomanip>

using namespace royale;
using namespace royale::common;
using namespace royale::importExportHelperLib;

CameraStatus royale::importExportHelperLib::encodeDepthCSV (const royale::DepthData *depthData, std::ostream &outCSV)
{
    // Create the data for the CSV-File
    outCSV << std::showpoint;
    outCSV << std::fixed;
    outCSV << std::setprecision (4);

    for (const auto &p : depthData->points)
    {
        outCSV << p.z << ',';
    }

    return CameraStatus::SUCCESS;
}

CameraStatus royale::importExportHelperLib::encodeDepthCSV (const royale::DepthData *depthData, std::string &outCSV)
{
    std::stringstream outBuffer{};
    auto returnCode = encodeDepthCSV (depthData, outBuffer);

    if (CameraStatus::SUCCESS == returnCode)
    {
        outCSV = outBuffer.str();
    }

    return returnCode;
}

royale::CameraStatus royale::importExportHelperLib::encodeAmplCSV (const royale::DepthData *depthData, std::string &outCSV)
{
    std::stringstream outBuffer{};
    auto returnCode = encodeAmplCSV (depthData, outBuffer);

    if (CameraStatus::SUCCESS == returnCode)
    {
        outCSV = outBuffer.str();
    }

    return returnCode;
}

royale::CameraStatus royale::importExportHelperLib::encodeAmplCSV (const royale::DepthData *depthData, std::ostream &outCSV)
{
    // Create the data for the CSV-File
    outCSV << std::showpoint;
    outCSV << std::fixed;
    outCSV << std::setprecision (4);

    for (const auto &p : depthData->points)
    {
        outCSV << static_cast<uint32_t> (p.grayValue) << ',';
    }

    return CameraStatus::SUCCESS;
}

royale::CameraStatus royale::importExportHelperLib::encodeRawCSV (const royale::RawData *rawData, std::ostream &outCSV)
{
    // Create the data for the CSV-File
    outCSV << std::showpoint;
    outCSV << std::fixed;
    outCSV << std::setprecision (4);

    for (size_t x = 0u; x < rawData->rawData.size(); ++x)
    {
        size_t idx = 0u;
        for (size_t i = 0; i < rawData->height; ++i)
        {
            for (size_t j = 0; j < rawData->width; ++j, ++idx)
            {
                outCSV << rawData->rawData.at (x) [idx] << ",";
            }
            outCSV << std::endl;
        }
        outCSV << std::endl;
        outCSV << std::endl;
    }

    return CameraStatus::SUCCESS;
}