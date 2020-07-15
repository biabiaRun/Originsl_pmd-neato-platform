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
#include <common/exceptions/Exception.hpp>

FileLog::FileLog (std::string logFilePath)
{
    fout.open (logFilePath.c_str(), std::ios::out);
    if (!fout.is_open())
    {
        throw royale::common::Exception ("open file for output failed!");
    }
}

FileLog::~FileLog()
{
    if (fout.is_open())
    {
        fout.close();
    }
}

void FileLog::printLogLine (uint16_t loglevel, const std::string &logLine)
{
    fout << std::dec << "[" << royale::common::LogSettings::getInstance()->getLogTime ("%Y/%m/%d %X").c_str()
         << " tid:" << std::this_thread::get_id() << "]" << " " << logLevToString (loglevel) << " " << logLine.c_str() << std::endl;
}
