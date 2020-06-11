/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <importExportHelperLib/imgHelper.hpp>

#include <common/StringFunctions.hpp>

#include <cmath>
#include <sstream>
#include <iomanip>

using namespace royale;
using namespace royale::common;
using namespace royale::importExportHelperLib;

CameraStatus royale::importExportHelperLib::encodeAmplPNG (const royale::DepthData *depthData, QImage &resultImg)
{
    static const int MAX_AMPLITUDE = 900;

    int maxWidth = depthData->width;
    int maxHeight = depthData->height;


    for (int curWidth = 0; curWidth < maxWidth; curWidth++)
    {
        for (int curHeight = 0; curHeight < maxHeight; curHeight++)
        {
            auto j = 70.f;
            int dataPos = curHeight * maxWidth + curWidth;
            uint16_t curAmpl = depthData->points[dataPos].grayValue;

            //16 bit(x) to 8 bit(y) :
            //y = 255 * log(1 + (j - 1)*x / 900) / log(j)
            float modAmplFloat = (log10f (1.0f + (j - 1.f) *  curAmpl / MAX_AMPLITUDE) / log10f (j));
            if (modAmplFloat > 1.0f)
            {
                modAmplFloat = 1.0f;
            }
            modAmplFloat = modAmplFloat * 255.0f;
            int modAmplInt = static_cast<int> (modAmplFloat);

            if (curWidth > resultImg.width() || curHeight > resultImg.height())
            {
                return royale::CameraStatus::OUT_OF_BOUNDS;
            }
            else
            {
                QColor qC (modAmplInt, modAmplInt, modAmplInt);
                resultImg.setPixel (curWidth, curHeight, qC.rgba());
            }
        }
    }

    return royale::CameraStatus::SUCCESS;
}

CameraStatus royale::importExportHelperLib::encodeIR1PNG (const DepthData *depthData, QImage &resultImg)
{
    int maxWidth = depthData->width;
    int maxHeight = depthData->height;

    for (int curWidth = 0; curWidth < maxWidth; curWidth++)
    {
        for (int curHeight = 0; curHeight < maxHeight; curHeight++)
        {
            int dataPos = curHeight * maxWidth + curWidth;
            uint8_t curAmpl = static_cast<uint8_t> (depthData->points[dataPos].grayValue);

            if (curWidth > resultImg.width() || curHeight > resultImg.height())
            {
                return royale::CameraStatus::OUT_OF_BOUNDS;
            }
            else
            {
                QColor qC (curAmpl, curAmpl, curAmpl);
                resultImg.setPixel (curWidth, curHeight, qC.rgba());
            }
        }
    }

    return royale::CameraStatus::SUCCESS;
}

CameraStatus royale::importExportHelperLib::encodeIR2PNG (const IntermediateData *intData, QImage &resultImg)
{
    int maxWidth = intData->width;
    int maxHeight = intData->height;

    for (int curWidth = 0; curWidth < maxWidth; curWidth++)
    {
        for (int curHeight = 0; curHeight < maxHeight; curHeight++)
        {
            int dataPos = curHeight * maxWidth + curWidth;
            uint8_t curAmpl = static_cast<uint8_t> (intData->points[dataPos].intensity);

            if (curWidth > resultImg.width() || curHeight > resultImg.height())
            {
                return royale::CameraStatus::OUT_OF_BOUNDS;
            }
            else
            {
                QColor qC (curAmpl, curAmpl, curAmpl);
                resultImg.setPixel (curWidth, curHeight, qC.rgba());
            }
        }
    }

    return royale::CameraStatus::SUCCESS;
}
