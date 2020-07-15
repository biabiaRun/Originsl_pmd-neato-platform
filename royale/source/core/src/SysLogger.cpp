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
#include <syslog.h>

void SysLogger::printLogLine (uint16_t loglevel, const std::string &logLine)
{
    switch ( (RoyaleLoggerLevels) loglevel)
    {
        case RoyaleLoggerLevels::NONE_:
            break;
        case RoyaleLoggerLevels::INFO_:
            syslog (LOG_INFO, "%s", logLine.c_str());
            break;
        case RoyaleLoggerLevels::DEBUG_:
            syslog (LOG_DEBUG, "%s", logLine.c_str());
            break;
        case RoyaleLoggerLevels::WARN_:
            syslog (LOG_WARNING, "%s", logLine.c_str());
            break;
        case RoyaleLoggerLevels::ERROR_:
            syslog (LOG_ERR, "%s", logLine.c_str());
            break;
        default:
            break;
    }
}

SysLogger::SysLogger()
{
    openlog (nullptr, LOG_PID, 0);
}

SysLogger::~SysLogger()
{

}
