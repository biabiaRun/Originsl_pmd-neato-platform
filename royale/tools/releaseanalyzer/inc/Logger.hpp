/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QFile>
#include <QString>
#include <QTextStream>
#include <iostream>
#include <cstdint>
#include <mutex>
#include <atomic>

/** Logging macro; to overcome multi macro definitions for levels
  * To create a Logentry which should be put out in two ore more cases use the follwing syntax:
  * Log((uint16_t)LoggerLevels::WARN_|(uint16_t)LoggerLevels::ERROR_)) << "LogMessage";
  */
#define LOG(X)    releaseanalyzer::Log((uint16_t)LoggerLevels::X##_, false) << " " << #X << " "
#define DLOG(X)   releaseanalyzer::Log((uint16_t)LoggerLevels::X##_, true) << " " << #X << " "

/** Definition of LogLevels
  * Loglevels are used as binary bitmask format an can be used together by using binary or
  * The set loglevel is binary and compared to the level required for the output
  */
enum class LoggerLevels : uint16_t
{
    NONE_ = 0x00,
    INFO_ = 0x01,
    DEBUG_ = 0x02,
    WARN_ = 0x04,
    ERROR_ = 0x08
};

namespace releaseanalyzer
{
    class LogSettings
    {
    public:
        /// Creates a global available instance (even creation is thread safe)
        static LogSettings *getInstance();

        /// Sets global Loglevel (there is a default definition of WARN_ and ERROR_)
        void setLogLevel (uint16_t logLevel);

        /// Can be used to pipe logging to a logfile (has to be called explicitly)
        void setLogFile (QString logFilePath);

        /// Returns the configured loglevel (the one from the class which defines what messages shall be created)
        uint16_t logLevel() const;

        /// Returns the current stream (might be "cout" or a file-stream if a logfile was specified before
        QFile *getLog();

        /// Returns the current LogTime (current time according to the given format)
        const std::string getLogTime (const std::string &dateTimeFormat) const;

        /// Explicitly define assignment operator as deleted, so prohibit copying
        LogSettings &operator= (const LogSettings &) = delete;

    private:
        LogSettings();

        /// Explicitly define copy ctor as deleted, so prohibit copying
        LogSettings (const LogSettings &) = delete;
        ~LogSettings();

        uint16_t m_logSetting;
        QFile *fout;
    };

    class Log
    {
    public:
        Log (uint16_t level = 0, bool logConsole = false);
        virtual ~Log();

        // default output
        template <typename T>
        Log &operator<< (const T &x)
        {
            if (m_output)
            {
                std::lock_guard<std::mutex> lock (m_acquisitionLock);

                if (LogSettings::getInstance()->getLog() != NULL)
                {
                    QTextStream (LogSettings::getInstance()->getLog()) << " " << x;
                }
                if (m_logConsole || LogSettings::getInstance()->getLog() != NULL)
                {
                    QTextStream (stdout) << " " << x;
                }

            }
            return *this;
        }

    private:
        bool m_output;
        bool m_endlLast;
        bool m_logConsole;
        std::mutex m_acquisitionLock;
    };
}
