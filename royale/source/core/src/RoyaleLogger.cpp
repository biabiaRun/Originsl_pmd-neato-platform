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
 ** the following syntax : LogSettings::getInstance()->setLogLevel(
 **                          LogSettings::getInstance()->logLevel() & (uint_16t)<LOGLEVEL>)
 ** where <LOGLEVEL> matches one of these levels:
 ** enum class RoyaleLoggerLevels : uint16_t {
 **   NONE_  = 0x00,
 **   INFO_  = 0x01,
 **   DEBUG_ = 0x02,
 **   WARN_  = 0x04,
 **   ERROR_ = 0x08
 ** };
**/

#include <common/RoyaleLogger.hpp>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <common/MakeUnique.hpp>

using namespace royale::common;
using namespace std::chrono;

namespace
{
    LogSettings s_instance;
    /// Mutex to separate log calls from different threads
    std::recursive_mutex g_loggerMutex;
}

Log::Log (uint16_t level) :
    // Normal customers should not be able to turn on the logging.
    // For debugging purposes during integration it is still helpful to have logging
    // available in Release mode.
#if defined(ROYALE_LOG_INTERNAL)
    m_output ( (LogSettings::getInstance()->logLevel() & level) > 0),
    m_endlLast (false),
    m_currentLogLevel (level)
#else
    m_output (false)
#endif
{
    m_logLock = std::unique_lock<std::recursive_mutex> (g_loggerMutex);
}

Log::~Log()
{
#if defined(ROYALE_LOG_INTERNAL)
    if (!m_endlLast && m_output)
    {
        operator<< (std::endl);
    }
#endif
}

Log &Log::operator<< (EndLine endlFunc)
{
    if (m_output)
    {
#if defined(ROYALE_LOG_INTERNAL)
        LogSettings::getInstance()->pushLogLine (m_currentLogLevel, m_Buffer.str());
        m_Buffer.str ("");
        // invoke
        m_endlLast = true;
#endif
    }
    return *this;
}

LogSettings *LogSettings::getInstance()
{
    return &s_instance;

    // Using a file-scope mutex here would't provide any thread safety.  If this function is called
    // before s_instance has been initialized then it is also being called before the mutex would be
    // initialized.
}

LogSettings::LogSettings() :
    m_logSetting (0)
{
#ifdef ROYALE_LOG_INTERNAL

#if defined (ROYALE_TARGET_PLATFORM_LINUX) || defined (ROYALE_TARGET_PLATFORM_APPLE)
#ifdef ROYALE_LOG_BACKEND_SYSLOG
    m_logBackend = common::makeUnique<SysLogger>();
#else
    m_logBackend = common::makeUnique<CommandLog>();
#endif
#endif

#ifdef TARGET_PLATFORM_ANDROID
    m_logBackend = common::makeUnique<AndroidLog>();
#endif

#ifdef ROYALE_TARGET_PLATFORM_WINDOWS
#ifdef ROYALE_LOGBACKEND_WIN_DEBUG_STRING_LOG
    m_logBackend = common::makeUnique<WindowsDebugStringLog>();
#else
    m_logBackend = common::makeUnique<CommandLog>();
#endif
#endif

#endif
    // We want to start without the info messages (ROYAL-2503).
    // The log level can be changed afterwards.
    m_logSetting |= (uint16_t) RoyaleLoggerLevels::DEBUG_;
    m_logSetting |= (uint16_t) RoyaleLoggerLevels::WARN_;
    m_logSetting |= (uint16_t) RoyaleLoggerLevels::ERROR_;
}

LogSettings::~LogSettings()
{

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

#if defined(ROYALE_TARGET_PLATFORM_WINDOWS)
    struct tm _timeStruct;
    localtime_s (&_timeStruct, &mytime);
#endif

    char mbstr[100];
    memset (&mbstr, 0, sizeof (mbstr));  // initialize to 0

#if defined(ROYALE_TARGET_PLATFORM_WINDOWS)
    std::strftime (mbstr, sizeof (mbstr), dateTimeFormat.c_str(), &_timeStruct);
#else
    std::strftime (mbstr, sizeof (mbstr), dateTimeFormat.c_str(), std::localtime (&mytime));
#endif
    return mbstr;
}

void LogSettings::setLogFile (std::string logfilePath)
{
    m_logBackend = common::makeUnique<FileLog> (logfilePath);
}

const void LogSettings::pushLogLine (uint16_t loglevel, const std::string &logLine)
{
    m_logBackend->printLogLine (loglevel, logLine);
}

std::unique_ptr<IlogBackend> LogSettings::swapLogBackend (std::unique_ptr<IlogBackend> backend)
{
    m_logBackend.swap (backend);
    return backend;
}

