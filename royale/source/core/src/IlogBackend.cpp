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

std::string IlogBackend::logLevToString (uint16_t logLev)
{
    switch ( (RoyaleLoggerLevels) logLev)
    {
        case RoyaleLoggerLevels::NONE_:
            return "NONE";
        case RoyaleLoggerLevels::INFO_:
            return "INFO";
        case RoyaleLoggerLevels::DEBUG_:
            return "DEBUG";
        case RoyaleLoggerLevels::WARN_:
            return "WARN";
        case RoyaleLoggerLevels::ERROR_:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

IlogBackend::IlogBackend()
{

}

IlogBackend::~IlogBackend()
{

}

