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

#ifdef TARGET_PLATFORM_ANDROID
#include <android/log.h>
#endif

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <thread>

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
    m_endlLast (false)
#else
    m_output (false)
#endif
#ifdef TARGET_PLATFORM_ANDROID
    , m_level (level)
#endif
{
    const static std::stringstream initial;

    m_logLock = std::unique_lock<std::recursive_mutex> (g_loggerMutex);

#ifdef TARGET_PLATFORM_ANDROID
    LogSettings::getInstance()->stream().str ("");
    LogSettings::getInstance()->stream().clear();
#else
    if (m_output)
    {
        LogSettings::getInstance()->stream().copyfmt (initial);
        LogSettings::getInstance()->stream() << std::dec << "[" <<
                                             LogSettings::getInstance()->getLogTime ("%Y/%m/%d %X").c_str() <<
                                             " tid:" << std::this_thread::get_id() <<
                                             "]";
    }
#endif
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
#if defined(ROYALE_LOG_INTERNAL)
    if (m_output)
    {
        // invoke
        m_endlLast = true;

#ifdef TARGET_PLATFORM_ANDROID
        switch ( (RoyaleLoggerLevels) m_level)
        {
            case RoyaleLoggerLevels::NONE_:
                __android_log_print (ANDROID_LOG_DEFAULT, "ROYALE", LogSettings::getInstance()->stream().str().c_str(), 1);
                break;
            case RoyaleLoggerLevels::INFO_:
                __android_log_print (ANDROID_LOG_INFO, "ROYALE", LogSettings::getInstance()->stream().str().c_str(), 1);
                break;
            case RoyaleLoggerLevels::DEBUG_:
                __android_log_print (ANDROID_LOG_DEBUG, "ROYALE", LogSettings::getInstance()->stream().str().c_str(), 1);
                break;
            case RoyaleLoggerLevels::WARN_:
                __android_log_print (ANDROID_LOG_WARN, "ROYALE", LogSettings::getInstance()->stream().str().c_str(), 1);
                break;
            case RoyaleLoggerLevels::ERROR_:
                __android_log_print (ANDROID_LOG_ERROR, "ROYALE", LogSettings::getInstance()->stream().str().c_str(), 1);
                break;
            default:
                __android_log_print (ANDROID_LOG_UNKNOWN, "ROYALE", LogSettings::getInstance()->stream().str().c_str(), 1);
        }
        LogSettings::getInstance()->stream().str ("");
        LogSettings::getInstance()->stream().clear();
#else
        endlFunc (LogSettings::getInstance()->stream());
#endif
    }
#endif
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
    // We want to start without the info messages (ROYAL-2503).
    // The log level can be changed afterwards.
    m_logSetting |= (uint16_t) RoyaleLoggerLevels::DEBUG_;
    m_logSetting |= (uint16_t) RoyaleLoggerLevels::WARN_;
    m_logSetting |= (uint16_t) RoyaleLoggerLevels::ERROR_;
}

LogSettings::~LogSettings()
{
    if (fout.is_open())
    {
        fout.close();
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

void LogSettings::setLogFile (std::string logFilePath)
{
    std::unique_lock<std::recursive_mutex> logLock (g_loggerMutex);

    // Write to File
    if (fout.is_open())
    {
        fout.close();
    }

    if (logFilePath.length() > 0)
    {
        fout.open (logFilePath.c_str(), std::ios::out);
        if (!fout.is_open())
        {
            std::cerr << "error: open file for output failed!" << std::endl;
        }
    }
}

#ifdef TARGET_PLATFORM_ANDROID
std::stringstream &LogSettings::stream()
{
    return m_androidLogout;
}
#else
std::ostream &LogSettings::stream()
{
    if (fout.is_open())
    {
        return fout;
    }
    else
    {
        return std::cout;
    }
}
#endif
