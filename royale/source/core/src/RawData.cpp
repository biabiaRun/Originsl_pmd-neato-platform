/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/RawData.hpp>

using namespace royale;

RawData::RawData()
    : width (0),
      height (0),
      illuminationTemperature (-273.15f)
{
}

RawData::RawData (size_t rawVectorSize)
    : width (0),
      height (0),
      illuminationTemperature (-273.15f)
{
    rawData.resize (rawVectorSize);
}
