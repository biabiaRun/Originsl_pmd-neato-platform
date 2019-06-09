/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <QStringList>

/**
 * This runs a program (with args) in a separate process.
 * The output can be stored in a logfile, and it's possible to specify a timeout.
 *
 */
class RunProgram
{
public:
    static int spawnvp (QStringList argv, const char *logfile = nullptr, int timeout = -1);

}; // class RunProgram
