/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

class ISystemInfo
{
public:
    virtual ~ISystemInfo() {}

    /**
    *  Calculate the CPU usage rate of process
    *
    *  @return the CPU usage rate of process
    */
    virtual double getProcCpuUsage() = 0;

protected:
    /**
    *  Get the number of processors
    *
    *  @return the number of processors
    */
    virtual int getProcessorsNumber() = 0;
};
