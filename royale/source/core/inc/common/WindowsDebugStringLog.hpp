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

#include "IlogBackend.hpp"

/**
* This Class takes care of the windows output logging.
* It is instantiated with Ilogbackend reference.
*/
class WindowsDebugStringLog : public IlogBackend
{
public:
    WindowsDebugStringLog();
    ~WindowsDebugStringLog();

    /// prints the log message as windows output debug string
    void printLogLine (uint16_t loglevel, const std::string &logLine) override;

};

