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

#include <royale/Definitions.hpp>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <mutex>
#include <atomic>

#if (!defined(NDEBUG) && defined(ROYALE_USE_LOGGING)) || defined(ROYALE_FORCE_LOGGING)
#define ROYALE_LOG_INTERNAL
#else
#undef ROYALE_LOG_INTERNAL
#endif

/** Logging macro; to overcome multi macro definitions for levels
  * To create a log entry which should be put out in two ore more cases use the following syntax:
  * Log((uint16_t)RoyaleLoggerLevels::WARN_|(uint16_t)RoyaleLoggerLevels::ERROR_)) << "LogMessage";
  * Don't do any calls/assignments inside the logging (e.g.
  * LOG(INFO) << "Start capture: " << ((cam->startCapture()==CameraStatus::SUCCESS)?"OK":"FAILED");)
  * as these will be removed when logging is disabled.
  *
  * Logging during static initialization (before main() is called) causes undefined behavior.
  */
#ifdef ROYALE_LOG_INTERNAL
#ifdef TARGET_PLATFORM_ANDROID
#define LOG(X)    royale::common::Log((uint16_t)RoyaleLoggerLevels::X##_)
#else
#define LOG(X)    royale::common::Log((uint16_t)RoyaleLoggerLevels::X##_) << " " << #X << " "
#endif
#else
#define LOG(X) if (true); else royale::common::Log((uint16_t)RoyaleLoggerLevels::X##_)
#endif

/** Definition of LogLevels
  * LogLevels are used as binary bit mask format and can be used together by using binary or.
  * The set log level is binary and compared to the level required for the output
  */
enum class RoyaleLoggerLevels : uint16_t
{
    NONE_ = 0x00,
    INFO_ = 0x01,
    DEBUG_ = 0x02,
    WARN_ = 0x04,
    ERROR_ = 0x08
};

namespace royale
{
    namespace common
    {
        class LogSettings
        {
        public:
            /// Note: to change the global logging settings, use the global instance from
            /// getInstance; the constructor is only public so that it can be used to create
            /// the global instance.
            LogSettings();
            ~LogSettings();

            /// Bitwise OR of all RoyaleLoggerLevels, use for setLogLevel (ENABLE_ALL_LOGS)
            static const RoyaleLoggerLevels ENABLE_ALL_LOGS = RoyaleLoggerLevels (0x0F);

            /// Retrieves the global instance
            static ROYALE_API LogSettings *getInstance();

            /// Sets global log level (there is a default definition of WARN_ and ERROR_)
            ROYALE_API void setLogLevel (uint16_t logLevel);

            /// Can be used to pipe logging to a log file (has to be called explicitly)
            ROYALE_API void setLogFile (std::string logfilePath);

            /// Returns the configured log level (the one from the class which defines what messages shall be created)
            ROYALE_API uint16_t logLevel() const;

#ifdef TARGET_PLATFORM_ANDROID
            /// Returns the current stream (for Android this is a stringstream)
            ROYALE_API std::stringstream &stream();
#else
            /// Returns the current stream (might be "cout" or a file-stream if a log file was specified before
            ROYALE_API std::ostream &stream();
#endif

            /// Returns the current LogTime (current time according to the given format)
            ROYALE_API const std::string getLogTime (const std::string &dateTimeFormat) const;

            /// Explicitly define assignment operator as deleted, so prohibit copying
            ROYALE_API LogSettings &operator= (const LogSettings &) = delete;

        private:
            /// Explicitly define copy ctor as deleted, so prohibit copying
            LogSettings (const LogSettings &) = delete;

#ifdef TARGET_PLATFORM_ANDROID
            std::stringstream m_androidLogout;
#endif

            uint16_t m_logSetting;
            std::ofstream fout;
        };

        class Log
        {
        public:
            // this is the signature for the std::endl function
            using EndLine = std::basic_ostream<char, std::char_traits<char> > & (*) (std::basic_ostream<char, std::char_traits<char> > &);

            ROYALE_API explicit Log (uint16_t level = 0);
            ROYALE_API virtual ~Log ();

            ROYALE_API Log &operator= (const Log &) = delete; // non copyable
            ROYALE_API Log (const Log &) = delete;

            // default output
            template <typename T>
            Log &operator<< (const T &x)
            {
#if defined(ROYALE_LOG_INTERNAL)
                if (m_output)
                {
                    m_endlLast = false;
                    LogSettings::getInstance()->stream() << x;
                }
#endif
                return *this;
            }

            // define an operator<< to take in std::endl
            Log &operator<< (EndLine endlFunc);

        private:
            bool m_output;
#if defined(ROYALE_LOG_INTERNAL)
            bool m_endlLast;
#endif
#ifdef TARGET_PLATFORM_ANDROID
            uint16_t m_level;
#endif

            std::unique_lock<std::recursive_mutex> m_logLock;
        };
    }
}
