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
#include <fstream>

/**
* This Class takes care of logging into a given file.
* It is instantiated with Ilogbackend reference.
*/
class FileLog : public IlogBackend
{
public:
    FileLog (std::string logFilePath);
    ~FileLog();

    /// prints the log message into the specified file
    void printLogLine (uint16_t loglevel, const std::string &logLine) override;

private:
    std::ofstream fout;
};
