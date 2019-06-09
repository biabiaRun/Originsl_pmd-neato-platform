/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

/** To change the standard LogLevel in any function you want to debug/log use
 ** the follwoing syntax : LogSettings::getInstance()->setLogLevel(
 **                          LogSettings::getInstance()->logLevel() & (uint_16t)<LOGLEVEL>)
 ** where <LOGLEVEL> matches one of the following levels:
 ** enum class LoggerLevels : uint16_t {
 **   NONE_ = 0x00,
 **   INFO_ = 0x01,
 **   DEBUG_ = 0x02,
 **   WARN_ = 0x04,
 **   ERROR_ = 0x08
 ** };
**/

#include <Logger.hpp>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <ctime>
#include <chrono>

using namespace releaseanalyzer;

namespace
{
    std::atomic<LogSettings *> m_instance;
    std::mutex m_mutex;
}

Log::Log (uint16_t level, bool logConsole) :
    // Do that for safety reasons - customer shall not be able to turn logging on!

    m_output ( (LogSettings::getInstance()->logLevel() & level) > 0)
    , m_logConsole (logConsole)
    , m_endlLast (false)
{
    if (m_output)
    {
        std::lock_guard<std::mutex> lock (m_acquisitionLock);
        if (LogSettings::getInstance()->getLog() != NULL)
        {
            QTextStream (LogSettings::getInstance()->getLog()) << "[" << LogSettings::getInstance()->getLogTime ("%Y/%m/%d %X").c_str() << "]";
        }
        /*if(m_logConsole || LogSettings::getInstance()->getLog() != NULL)
        {
            QTextStream(stdout) << "[" << LogSettings::getInstance()->getLogTime ("%Y/%m/%d %X").c_str() << "]";
        }*/
    }
}

Log::~Log()
{
    if (!m_endlLast && m_output)
    {
        std::lock_guard<std::mutex> lock (m_acquisitionLock);

        if (LogSettings::getInstance()->getLog() != NULL)
        {
            QTextStream (LogSettings::getInstance()->getLog()) << "\r\n";
        }
        if (m_logConsole || LogSettings::getInstance()->getLog() != NULL)
        {
            QTextStream (stdout) << "\r\n";
        }
    }
}

LogSettings *LogSettings::getInstance()
{
    LogSettings *tmp = m_instance.load (std::memory_order_relaxed);
    std::atomic_thread_fence (std::memory_order_acquire);
    if (tmp == nullptr)
    {
        std::lock_guard<std::mutex> lock (m_mutex);
        tmp = m_instance.load (std::memory_order_relaxed);
        if (tmp == nullptr)
        {
            tmp = new LogSettings;
            std::atomic_thread_fence (std::memory_order_release);
            m_instance.store (tmp, std::memory_order_relaxed);
        }
    }
    return tmp;
}

LogSettings::LogSettings() :
    m_logSetting (0)
{
    m_logSetting |= (uint16_t) LoggerLevels::INFO_;
    m_logSetting |= (uint16_t) LoggerLevels::DEBUG_;
    m_logSetting |= (uint16_t) LoggerLevels::WARN_;
    m_logSetting |= (uint16_t) LoggerLevels::ERROR_;
}

LogSettings::~LogSettings()
{
    if (fout->isOpen())
    {
        fout->close();
        delete fout;
    }
}

void LogSettings::setLogLevel (uint16_t logSetting)
{
    m_logSetting = logSetting;
}

uint16_t LogSettings::logLevel() const
{
    return m_logSetting;
}

const std::string LogSettings::getLogTime (const std::string &dateTimeFormat) const
{
    time_t mytime;
    mytime = time (NULL);

#if defined(_WINDOWS)
    struct tm _timeStruct;
    localtime_s (&_timeStruct, &mytime);
#endif

    char mbstr[100];
    memset (&mbstr, 0, sizeof (mbstr));  // initialize to 0

#if defined(_WINDOWS)
    std::strftime (mbstr, sizeof (mbstr), dateTimeFormat.c_str(), &_timeStruct);
#else
    std::strftime (mbstr, sizeof (mbstr), dateTimeFormat.c_str(), std::localtime (&mytime));
#endif
    return mbstr;
}

void LogSettings::setLogFile (QString logFilePath)
{
    // Write to File
    fout = new QFile (logFilePath);
    fout->open (QIODevice::Append);
    if (!fout->isOpen())
    {
        std::cerr << "error: open file for output failed!" << std::endl;
    }
}

QFile *LogSettings::getLog ()
{
    return fout;
}
