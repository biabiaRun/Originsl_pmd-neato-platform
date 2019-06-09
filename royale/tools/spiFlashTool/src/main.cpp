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
#include <SPIFlashTool.hpp>

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);
    app.setOrganizationName ("pmdtechnologies ag & Infineon Technologies AG");
    app.setApplicationName ("SPIFlashTool");

    QCommandLineParser parser;
    parser.setApplicationDescription ("SPIFlashTool");

    // add help options
    parser.addHelpOption();

    // add optional parameters
    parser.addOptions (
    {
        { "cal", "calibration file to flash", "calibrationFile.jgf" },
        { "serial", "module serial", "1234" },
        { "ident", "product code (hex)", "01033001004050001000000407000000" },
        { "suffix", "module suffix", "850nm" }
    });

    // parse the command line
    parser.process (app);

    SPIFlashToolParameters params;

    params.calibrationFilename = parser.value ("cal").toLocal8Bit().constData();
    params.moduleSerialNumber = parser.value ("serial").toLocal8Bit().constData();
    params.moduleIdentifierHex = parser.value ("ident").toLocal8Bit().constData();
    params.moduleSuffix = parser.value ("suffix").toLocal8Bit().constData();

    SPIFlashTool spiFlashTool (params);
    spiFlashTool.show();
    return app.exec();
}
