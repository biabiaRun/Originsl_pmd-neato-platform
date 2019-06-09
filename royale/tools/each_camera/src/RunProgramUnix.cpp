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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>

int
RunProgram::spawnvp (QStringList argv, const char *logfile, int timeout)
{
    // Convert args back to C-style argv
    std::vector<std::string> args_strings;
    std::vector<char *> args_data;
    for (const auto &a : argv)
    {
        args_strings.emplace_back (a.toStdString());
    }
    for (const auto &a : args_strings)
    {
        args_data.emplace_back (const_cast<char *> (a.c_str()));
    }
    args_data.emplace_back (nullptr);
    auto my_argv = args_data.data();

    auto pid = fork();

    switch (pid)
    {
        case 0: // child process
            if (logfile)
            {
                auto fd = open (logfile, O_CREAT | O_WRONLY, 0660);
                if (fd < 0)
                {
                    std::cerr << "Can't open logfile" << std::endl;
                    exit (1); // we're child...
                }

                // redirect stdout/stderr
                dup2 (fd, 1);
                dup2 (fd, 2);
                close (fd);
            }

            if (timeout > 0)
            {
                alarm (timeout);
            }
            execvp (my_argv[0], my_argv);
            // If we reach this, execvp() has failed
            std::cerr << "Can't start " << my_argv[0] << std::endl;
            exit (1);
        case -1: // error (fork failed; this is still the parent process)
            std::cerr << "Failed to create process for " << my_argv[0] << std::endl;
            return -1;
        default: // parent needs to wait for child
            waitpid (pid, nullptr, 0);
            break;
    }
    return 0;
}
