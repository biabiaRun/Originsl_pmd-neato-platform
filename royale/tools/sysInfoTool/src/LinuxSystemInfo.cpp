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
#include "LinuxSystemInfo.hpp"

using namespace std;

LinuxSystemInfo::LinuxSystemInfo()
{
#ifdef TARGET_PLATFORM_ANDROID
    m_processorsNumber = 1;
#else
    m_processorsNumber = getProcessorsNumber();
#endif
}

LinuxSystemInfo::~LinuxSystemInfo()
{
}

int LinuxSystemInfo::getProcessorsNumber()
{
    FILE *file;
    char buf[BUF_SIZE];
    int processorsNumber = 0;

    file = fopen ("/proc/cpuinfo", "r");
    while (fgets (buf, BUF_SIZE, file) != NULL)
    {
        if (strncmp (buf, "processor", 9) == 0)
        {
            processorsNumber++;
        }
    }
    fclose (file);
    return processorsNumber;
}

double LinuxSystemInfo::getProcCpuUsage ()
{
    static unsigned long long preUser, preNice, preSys, preIdle, preIowait, preIrq, preSoftirq, preStealstolen, preGuest;
    unsigned long long user, nice, sys, idle, iowait, irq, softirq, stealstolen, guest;

    FILE *cpuFile;
    cpuFile = fopen ("/proc/stat", "r");
    int n = fscanf (cpuFile, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &sys, &idle, &iowait, &irq, &softirq, &stealstolen, &guest);
    if (n != 9)
    {
        printf ("Get time of CPU Failed!");
    }
    fclose (cpuFile);

    unsigned long long preCpuTime = preUser + preNice + preSys + preIdle + preIowait + preIrq + preSoftirq + preStealstolen + preGuest;
    unsigned long long cpuTime = user + nice + sys + idle + iowait + irq + softirq + stealstolen + guest;
    double totalCpuTime = (double) (cpuTime - preCpuTime);

    preUser = user;
    preNice = nice;
    preSys = sys;
    preIdle = idle;
    preIowait = iowait;
    preIrq = irq;
    preSoftirq = softirq;
    preStealstolen = stealstolen;
    preGuest = guest;

    static unsigned long long preUtime, preStime, preCutime, preCstime;
    unsigned long long utime, stime, cutime, cstime;

    FILE *procFile;
    char line[BUF_SIZE];

    procFile = fopen ("/proc/self/stat", "r");
    static_assert (BUF_SIZE > 500, "BUF_SIZE too small, please adjust format string in the following line");
    n = fscanf (procFile, "%500[^\n]%*c", line);
    if (n != 1)
    {
        printf ("Get time of process Failed!");
    }
    fclose (procFile);

    int i = 0;
    char *part;
    part = strtok (line, " ");
    while (part && i < 13)
    {
        part = strtok (NULL, " ");
        i++;
    }
    if (part == nullptr)
    {
        return 0.0;
    }
    utime = atoll (part);
    part = strtok (NULL, " ");
    if (part == nullptr)
    {
        return 0.0;
    }
    stime = atoll (part);
    part = strtok (NULL, " ");
    if (part == nullptr)
    {
        return 0.0;
    }
    cutime = atoll (part);
    part = strtok (NULL, " ");
    if (part == nullptr)
    {
        return 0.0;
    }
    cstime = atoll (part);

    unsigned long long preProcTime = preUtime + preStime + preCutime + preCstime;
    unsigned long long procTime = utime + stime + cutime + cstime;
    double procCpuUsage = (double) (procTime - preProcTime) * 100.0 / totalCpuTime * m_processorsNumber;

    preUtime = utime;
    preStime = stime;
    preCutime = cutime;
    preCstime = cstime;

    return procCpuUsage;
}

#endif
