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

#include <string>
#include <cstdint>

/**
* Log backend Interface Class
* This is the interface used as parent reference for
* creating child instances of different logging mechanism.
*/
class IlogBackend
{
public:
    IlogBackend();
    virtual ~IlogBackend();

    /// to get log level as string
    std::string logLevToString (uint16_t logLev);

    /// prints the log message
    virtual void printLogLine (uint16_t loglevel, const std::string &logLine) = 0;
};
