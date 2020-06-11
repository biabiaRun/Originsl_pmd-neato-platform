/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <common/RoyaleTime.hpp>


using namespace royale::common;
using namespace std::chrono;

TimeMeasurement::TimeMeasurement (std::string measurementName)
{
    m_startTime = std::chrono::system_clock::now();
    m_measurementName = measurementName;
}
TimeMeasurement::~TimeMeasurement()
{
    std::ostringstream out;
    auto result = duration_cast<microseconds> (std::chrono::system_clock::now() - m_startTime);
    out << result.count();
    std::string outString = out.str();

    LOG (DEBUG) << "\"" << m_measurementName << "\" : " << outString << " microseconds.";
}