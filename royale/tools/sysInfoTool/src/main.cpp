/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QCommandLineParser>
#include <string>
#include "SysInfoTool.hpp"

using namespace std;

int main (int argc, char *argv[])
{
    QCoreApplication app (argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription ("System Information Tool");
    parser.addHelpOption();
    parser.addOptions (
    {
        { "rrf", "rrf file to open", "recording.rrf" },
        { "dir", "target folder to search rrf file", "folder path", "" },
        { "sec", "playback time to replay rrf file", "seconds", "" }
    });
    parser.process (app);
    string rrfFile = parser.value ("rrf").toStdString();
    string findFilePath = parser.value ("dir").toStdString();
    int seconds = parser.value ("sec").toInt();

    SysInfoTool sysInfoTool (rrfFile, findFilePath, seconds);

    return app.exec();
}
