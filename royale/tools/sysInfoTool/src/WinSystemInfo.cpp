/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#ifdef WIN32
#include "WinSystemInfo.hpp"

using namespace std;

WinSystemInfo::WinSystemInfo()
{
    m_processorsNumber = getProcessorsNumber();
    m_processID = getProcessID();
}

WinSystemInfo::~WinSystemInfo()
{
}

int WinSystemInfo::getProcessorsNumber()
{
    SYSTEM_INFO info;
    GetSystemInfo (&info);
    return (int) info.dwNumberOfProcessors;
}

int WinSystemInfo::getProcessID()
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    int processID;

    hProcessSnap = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "CreateToolhelp32Snapshot Failed!" << endl;
        if (NULL != hProcessSnap)
        {
            CloseHandle (hProcessSnap);
            hProcessSnap = NULL;
        }
    }

    pe32.dwSize = sizeof (PROCESSENTRY32);
    if (!Process32First (hProcessSnap, &pe32))
    {
        cout << "Process32First Failed!" << endl;
        if (NULL != hProcessSnap)
        {
            CloseHandle (hProcessSnap);
            hProcessSnap = NULL;
        }
    }

    do
    {
        string processName = string (pe32.szExeFile);
        if (processName == "sysInfoTool.exe")
        {
            processID = (int) pe32.th32ProcessID;
            break;
        }
    }
    while (Process32Next (hProcessSnap, &pe32));

    if (NULL != hProcessSnap)
    {
        CloseHandle (hProcessSnap);
        hProcessSnap = NULL;
    }
    return processID;
}

double WinSystemInfo::getProcCpuUsage ()
{
    static FILETIME preProcKernelTime;
    static FILETIME preProcUserTime;
    static FILETIME preSysTime;

    FILETIME sysTime;
    FILETIME creatTime;
    FILETIME exitTime;
    FILETIME procKernelTime;
    FILETIME procUserTime;

    HANDLE hP = OpenProcess (PROCESS_ALL_ACCESS, FALSE, m_processID);
    GetSystemTimeAsFileTime (&sysTime);
    GetProcessTimes (hP, &creatTime, &exitTime, &procKernelTime, &procUserTime);
    CloseHandle (hP);

    __int64 procKernel = compareFileTime (preProcKernelTime, procKernelTime);
    __int64 procUser = compareFileTime (preProcUserTime, procUserTime);
    __int64 sysProc = compareFileTime (preSysTime, sysTime);

    double procCpuUsage = (procKernel + procUser) * 100.0 / sysProc / m_processorsNumber;

    preProcKernelTime = procKernelTime;
    preProcUserTime = procUserTime;
    preSysTime = sysTime;

    return procCpuUsage;
}

__int64 WinSystemInfo::compareFileTime (FILETIME preTime, FILETIME nowTime)
{
    __int64 a, b;
    a = static_cast<__int64> (preTime.dwHighDateTime) << 32 | preTime.dwLowDateTime;
    b = static_cast<__int64> (nowTime.dwHighDateTime) << 32 | nowTime.dwLowDateTime;

    return (b - a);
}

#endif