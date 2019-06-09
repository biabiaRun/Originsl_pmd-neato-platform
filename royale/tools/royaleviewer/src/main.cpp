/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <QApplication>
#include <QPair>
#include <royale.hpp>
#ifndef QT_NO_OPENGL
#include "qtviewer.hpp"
#endif

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);
    app.setOrganizationName ("pmdtechnologies ag & Infineon Technologies AG");
    app.setApplicationName ("Royale Viewer");
    QSurfaceFormat fmt;
    fmt.setVersion (4, 4);
    fmt.setDepthBufferSize (24);

    QCommandLineParser parser;
    parser.setApplicationDescription ("Royale Viewer");

    // add help options
    parser.addHelpOption();

    // add access code as positional argument (for backwards compatibility)
    parser.addPositionalArgument ("accessCode", "code to gain higher level access rights (this is deprecated, please use the 'code' parameter)");
    parser.addPositionalArgument ("path", "the path to a RRF-file");

    // add optional parameters
    parser.addOptions (
    {
        { "rrf", "rrf file to open", "recording.rrf" },
        { "cal", "alternate calibration file to use", "calibrationFile.jgf" },
        { "ac", "auto connect", "", "" },
        { "mode", "use case", "defaultUseCase", "" },
        { "ae", "auto exposure", "", "" },
        { "slave", "open camera as slave", "", "" },
        { "code", "access code", "code", "" }
    });

    // parse the command line
    parser.process (app);

    QTViewerParameters params;
    // retrieve access code (if any)
    // retrieve file location (if any)
    const QStringList args = parser.positionalArguments();
    for (QString arg : parser.positionalArguments())
    {
        if (arg.endsWith (".rrf"))
        {
            params.playbackFilename = arg.toLocal8Bit().constData();
        }
        else
        {
            params.accessCode = arg.toStdString();
            std::cout << "Warning: arguments that don't end with .rrf are treated as access codes, not filenames." << std::endl;
            std::cout << "(If you intended it to be an access code, using the --code option will avoid this warning)" << std::endl;
        }
    }

    // retrieve other command line parameters
    if (parser.isSet ("rrf") &&
            params.playbackFilename.empty())
    {
        params.playbackFilename = parser.value ("rrf").toLocal8Bit().constData();
    }
    params.calibrationFileName = parser.value ("cal").toLocal8Bit().constData();
    params.autoConnect = parser.isSet ("ac");
    params.startUseCase = parser.value ("mode");
    params.autoExposure = parser.isSet ("ae");
    params.cameraSlave = parser.isSet ("slave");
    if (parser.isSet ("code") &&
            params.accessCode.empty())
    {
        params.accessCode = parser.value ("code").toStdString();
    }

    qRegisterMetaType<uint16_t> ("uint16_t");
    qRegisterMetaType<royale::StreamId> ("royale::StreamId");


#ifndef QT_NO_OPENGL

#ifndef ROYALE_TARGET_PLATFORM_ANDROID
    app.setAttribute (Qt::AA_UseDesktopOpenGL);
#endif


    QTViewer viewer (params);
    QObject::connect (&app, SIGNAL (applicationStateChanged (Qt::ApplicationState)), &viewer, SLOT (onApplicationStateChanged (Qt::ApplicationState)));
    viewer.show();
#else
    QLabel note ("OpenGL required");
    note.show();
#endif
    return app.exec();
}
