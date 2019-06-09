/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "RunProgram.hpp"
#include "CameraBlocker.hpp"

#include <QCommandLineParser>
#include <QApplication>

#include <iostream>
#include <string>
#include <set>
#include <iomanip>
#include <ctime>
#include <chrono>

#if defined(_WINDOWS)
#include <windows.h>
#endif

// Helper: Get current system time in ISO-ISO-8601 format (Zulu time)
std::string
getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t (now);

    char buf[17];

#if _POSIX_C_SOURCE >=1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE || _POSIX_SOURCE
    // SUSv2 (i.e. Linux and others) have gmtime_r, which is a thread-safe replacement for gmtime.
    std::tm tm_buf;
    gmtime_r (&now_c, &tm_buf);
    auto tm_ptr = &tm_buf;
#elif defined(_WINDOWS)
    // MSVC has gmtime_s, which is another thread-safe replacement for gmtime.
    std::tm tm_buf;
    gmtime_s (&tm_buf, &now_c);
    auto tm_ptr = &tm_buf;
#else
    // This may be not thread-safe.
    auto tm_ptr = gmtime (&now_c);
#endif

    std::strftime (buf, sizeof (buf), "%Y%m%dT%H%M%SZ", tm_ptr);

    return buf;
}

int main (int argc, char *argv[])
{
    std::set<std::string> camera_list;

    QCommandLineParser cmdLineParser;
    QApplication dummyApp (argc, argv); // used for command line parsing only

    {
        CameraBlocker cb;

        dummyApp.setApplicationName ("each_camera");

#define VERSION_TO_STRING(MAJOR,MINOR,PATCH) #MAJOR "." #MINOR "." #PATCH
        dummyApp.setApplicationVersion (VERSION_TO_STRING (ROYALE_VERSION_MAJOR, ROYALE_VERSION_MINOR, ROYALE_VERSION_PATCH));
        cmdLineParser.setApplicationDescription ("run a command for each connected Royale camera");
        cmdLineParser.addHelpOption();
        cmdLineParser.addVersionOption();
        cmdLineParser.addOptions (
        {
            { "l", "Log stdout/stderr to file" },
            { "t", "Timeout <timeout in s>", "timeout" },
            { "c", "Camera <camera id>", "cameras" }
        });

        cmdLineParser.process (dummyApp);

        auto do_logging = cmdLineParser.isSet ("l");
        int timeout = -1;

        if (cmdLineParser.isSet ("t"))
        {
            timeout = atoi (cmdLineParser.value ("t").toStdString().c_str());
        }

        if (cmdLineParser.isSet ("c"))
        {
            for (const auto &a : cmdLineParser.values ("c"))
            {
                camera_list.emplace (a.toStdString());
            }
        }
        else
        {
            for (const auto &a : cb)
            {
                camera_list.emplace (a.name());
            }
        }

        auto args = cmdLineParser.positionalArguments();

        for (auto camera_name : camera_list)
        {
            std::cout << camera_name << std::endl;

            auto a = cb.find (camera_name);
            if (a == cb.end())
            {
                std::cerr << "No such camera: " << camera_name << std::endl;
            }
            else
            {
                a.unlock();
                if (!args.empty())
                {
                    std::string logfile;
                    if (do_logging)
                    {
                        auto tsAsString = getTimeStamp();
                        logfile = "log-" + tsAsString + "-" + camera_name + ".txt";
                    }

                    RunProgram::spawnvp (args, do_logging ? logfile.c_str() : nullptr, timeout);
                }
                a.lock();

            }

        }
    }
}
