/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#ifdef __linux__  // incl. Linux and Android
#pragma once

#include "ISystemInfo.hpp"
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>

#define BUF_SIZE 1024

class LinuxSystemInfo : public ISystemInfo
{
public:
    LinuxSystemInfo();
    virtual ~LinuxSystemInfo();

    double getProcCpuUsage() override;

protected:

    int getProcessorsNumber() override;

private:
    int m_processorsNumber;
};

#endif
