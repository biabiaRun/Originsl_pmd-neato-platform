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
* This Class takes care of the command line logging.
* It is instantiated with Ilogbackend reference.
*/
class CommandLog : public IlogBackend
{
public:
    CommandLog();
    ~CommandLog();

    /// prints the log message on the command line
    void printLogLine (uint16_t loglevel, const std::string &logLine) override;
};
