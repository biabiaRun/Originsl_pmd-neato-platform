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

#include <royale/Definitions.hpp>
#include "RoyaleLogger.hpp"

#include <chrono>
#include <string>
#include <sstream>
#include <iostream>


// insert into code to start the time measurement
// uses the MeasurementName as the object name
#ifdef ROYALE_ENABLE_TIME_MEASUREMENTS
#define ADD_TIME_MEASUREMENT(MeasurementName) royale::common::TimeMeasurement _MeasurementName(#MeasurementName);
#else
#define ADD_TIME_MEASUREMENT(MeasurementName)
#endif

namespace royale
{
    namespace common
    {
        class TimeMeasurement
        {
        public:
            TimeMeasurement (std::string measurementName);
            ~TimeMeasurement();

        private:
            std::chrono::time_point<std::chrono::system_clock> m_startTime;
            std::string m_measurementName;
        };
    }
}
