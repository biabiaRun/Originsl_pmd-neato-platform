/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/SensorMap.hpp>
#include <common/exceptions/InvalidValue.hpp>

using namespace royale::common;

const SensorMap::mapped_type royale::common::findSensorRoute (const SensorMap &sensorMap, SensorRole role)
{
    auto sensorIter = sensorMap.find (role);
    if (sensorIter == sensorMap.end())
    {
        return nullptr;
    }
    return sensorIter->second;
}
