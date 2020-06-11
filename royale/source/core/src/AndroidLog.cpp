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
#include <android/log.h>

void AndroidLog::printLogLine (uint16_t loglevel, const std::string &logLine)
{
    switch ( (RoyaleLoggerLevels) loglevel)
    {
        case RoyaleLoggerLevels::NONE_:
            __android_log_print (ANDROID_LOG_DEFAULT, "ROYALE", logLine.c_str(), 1);
            break;
        case RoyaleLoggerLevels::INFO_:
            __android_log_print (ANDROID_LOG_INFO, "ROYALE", logLine.c_str(), 1);
            break;
        case RoyaleLoggerLevels::DEBUG_:
            __android_log_print (ANDROID_LOG_DEBUG, "ROYALE", logLine.c_str(), 1);
            break;
        case RoyaleLoggerLevels::WARN_:
            __android_log_print (ANDROID_LOG_WARN, "ROYALE", logLine.c_str(), 1);
            break;
        case RoyaleLoggerLevels::ERROR_:
            __android_log_print (ANDROID_LOG_ERROR, "ROYALE", logLine.c_str(), 1);
            break;
        default:
            __android_log_print (ANDROID_LOG_UNKNOWN, "ROYALE", logLine.c_str(), 1);
    }
}

AndroidLog::AndroidLog()
{

}

AndroidLog::~AndroidLog()
{

}
