/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/RoyaleLogger.hpp>


void CommandLog::printLogLine (uint16_t loglevel, const std::string &logLine)
{
    std::cout << std::dec << "[" << royale::common::LogSettings::getInstance()->getLogTime ("%Y/%m/%d %X").c_str()
              << " tid:" << std::this_thread::get_id() << "]" << " " << logLevToString (loglevel) << " " << logLine.c_str() << std::endl;
}

CommandLog::CommandLog()
{

}

CommandLog::~CommandLog()
{

}
