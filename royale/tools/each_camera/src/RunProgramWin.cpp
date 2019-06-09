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

#include <windows.h>
#include <shlwapi.h>

#include <iostream>
#include <vector>

int
RunProgram::spawnvp (QStringList argv, const char *logfile, int timeout)
{
    if (argv.empty())
    {
        std::cerr << "No command given." << std::endl;
        return -1;
    }

    auto loghnd = INVALID_HANDLE_VALUE;

    STARTUPINFO suinfo; // startupinfo is used for i/o redirection (if needed)
    ZeroMemory (&suinfo, sizeof (suinfo));
    suinfo.cb = sizeof (suinfo);

    // CreateProcess doesn't honor PATH; MSDN suggests ShellExecute as alternative,
    // but that is completely useless.
    // So we need to search by ourselves.
    char cmd[MAX_PATH];
    auto argv0_as_string = argv.at (0).toStdString();
    strcpy_s (cmd, MAX_PATH, argv0_as_string.c_str());

    const char *extra_paths[] = { ".", nullptr };

    if (!PathFindOnPath (cmd, extra_paths))
    {

        if (strlen (cmd) >= MAX_PATH - 4)
        {
            std::cerr << "Can't find " << argv0_as_string << std::endl;
            return -1;
        }
        // also try with .exe tacked on.
        strcat_s (cmd, MAX_PATH, ".EXE");
        if (!PathFindOnPath (cmd, extra_paths))
        {
            std::cerr << "Can't find " << argv0_as_string << " or " << argv0_as_string << ".EXE" << std::endl;
            return -1;
        }
    }

    // Try reconstructing the command line...
    std::string cmdline_string;
    for (auto a : argv)
    {
        if (!cmdline_string.empty())
        {
            cmdline_string += " ";
        }
        cmdline_string += a.toStdString();
    }
    // CreateProcess (below) expects char*, but c_str() gives const char*.
    // We could probably get away with a const_cast here, but let's not set a bad example.
    std::vector<char> cmdline_vector (cmdline_string.begin(), cmdline_string.end());
    cmdline_vector.push_back ('\0');
    auto cmdline = cmdline_vector.data();


    if (logfile)
    {
        // Open an inheritable file handle for the log file and put it into the startupinfo
        SECURITY_ATTRIBUTES sattrs;
        ZeroMemory (&sattrs, sizeof (sattrs));
        sattrs.nLength = sizeof (sattrs);
        sattrs.bInheritHandle = TRUE;
        sattrs.lpSecurityDescriptor = NULL;

        loghnd = CreateFile (
                     logfile,
                     GENERIC_READ | GENERIC_WRITE, // read access isn't needed, but the docs recommend it...
                     FILE_SHARE_READ,
                     &sattrs,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);

        if (loghnd == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Can't create log file " << logfile << std::endl;
            return -1;
        }

        suinfo.hStdError = loghnd;
        suinfo.hStdOutput = loghnd;
        suinfo.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
        suinfo.dwFlags |= STARTF_USESTDHANDLES;
    }

    // Start the process.
    PROCESS_INFORMATION procinfo;
    ZeroMemory (&procinfo, sizeof (procinfo));
    if (CreateProcess (cmd, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &suinfo, &procinfo) == 0)
    {
        std::cerr << "Can't start process " << argv0_as_string << std::endl;
        if (loghnd != INVALID_HANDLE_VALUE)
        {
            CloseHandle (loghnd);
        }
        return -1;
    }

    // ...and wait for process termination
    auto ret = WaitForSingleObject (procinfo.hProcess, (timeout >= 0) ? (timeout * 1000) : INFINITE);

    if (ret == WAIT_TIMEOUT)
    {
        // Terminate child process (with extreme prejudice). May cause trouble, e.g. due to a camera
        // not being closed correctly (which could cause other cameras to fail...).
        TerminateProcess (procinfo.hProcess, 1);
        WaitForSingleObject (procinfo.hProcess, INFINITE);

        if (loghnd != INVALID_HANDLE_VALUE)
        {
            auto msg = "Terminated\n";
            DWORD dummy;
            WriteFile (loghnd, msg, static_cast<DWORD> (strlen (msg)), &dummy, NULL);
        }
    }

    CloseHandle (procinfo.hProcess);
    CloseHandle (procinfo.hThread);
    if (loghnd != INVALID_HANDLE_VALUE)
    {
        CloseHandle (loghnd);
    }

    return 0;
}
