/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <QApplication>
#include <ZwetschgeFlashTool.hpp>

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);
    app.setOrganizationName ("pmdtechnologies ag & Infineon Technologies AG");
    app.setApplicationName ("ZwetschgeFlashTool");

    QCommandLineParser parser;
    parser.setApplicationDescription ("ZwetschgeFlashTool");

    // add help options
    parser.addHelpOption();

    // add optional parameters
    parser.addOptions (
    {
        { "m2455", "Assume that the imager is an M2455 (MiraDonna). Normally auto-detected by name." },
        { "zwetschge", "Zwetschge file to flash", "dummy.zwetschge" },
        { "calibout", "filename of calib to be written", "dummy.jgf" },
    });

    // parse the command line
    parser.process (app);

    ZwetschgeFlashToolParameters params;

    params.forceM2455 = parser.isSet ("m2455");
    params.zwetschgeFilename = parser.value ("zwetschge").toLocal8Bit().constData();
    params.calibFilename = parser.value ("calibout").toLocal8Bit().constData();

    ZwetschgeFlashTool zwetschgeFlashTool (params);
    zwetschgeFlashTool.show();
    return app.exec();
}
