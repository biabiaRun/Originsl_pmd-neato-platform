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
#pragma once

#include "ISystemInfo.hpp"
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Psapi.lib")
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>

class WinSystemInfo : public ISystemInfo
{
public:
    WinSystemInfo();
    virtual ~WinSystemInfo();

    double getProcCpuUsage() override;

protected:

    int getProcessorsNumber() override;

    /**
    *  Traverse all processes and get the process ID through the process name
    *  The process name here is "sysInfoTool.exe".
    *
    *  @return the process ID of "sysInfoTool.exe"
    */
    int getProcessID();

    /**
    *  Subtraction operation for two filetimes
    *
    *  @param preTime: the previous filetime for previous frame of the DepthData
    *  @param nowTime: the current filetime
    *
    *  @return the difference between the two filetimes
    */
    __int64 compareFileTime (FILETIME preTime, FILETIME nowTime);

private:
    int   m_processorsNumber;
    int   m_processID;
};

#endif
