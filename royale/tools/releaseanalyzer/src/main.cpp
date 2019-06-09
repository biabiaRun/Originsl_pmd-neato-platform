/****************************************************************************
**                                                                        **
**  Copyright (C) 2015 Infineon Technologies                              **
**  All rights reserved.                                                  **
**                                                                        **
****************************************************************************/

/**
**
** Creating Platform reference
**  Necessary to create a reference file for later checking the integrity after unpack
**
** releaseanalyzer.exe -createReference "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit" expectedStructureLINUX64.txt

**
** Unpacking and checking a package
**  package stecifies the package the analyzer is going to extract
**  unpackpath is the path where the package will be unpacked
**  checkPath is the path to which the package was unpacked (may differ from unpackpath; depending on how the archive was created ... with a root folder or not)
**
** releaseanalyzer.exe
**  -package "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit.tar.gz"
**  -unpackpath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
**  -checkPath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
**  -debug

**
** Validation against expectation
**  validationFileName specifies the file which helds the necessary expectation
**   This is normally the file from step 1 without unnecessary directories and sub directories; these will be excluded from checking
**   If there were any changes in the reference file from step 1 and complete directory structures were removed from checking, then do not
**   forget to provide -ignorepathandsubs PATH as many times as necessary to exclude these paths from checking to avoid error flooding.
**  checkPath has to be give to tell the analyzer which directory structure it shall compare to the one in the validationFile
**
** releaseanalyzer.exe
**  -validationFileName expectedStructureLINUX64.txt
**  -checkPath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
**  -ignorepathandsubs doc
**  -differenceFile DifferencefileName
**  -debug
**/

#include <PackageHandler.hpp>
#include <Logger.hpp>

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QApplication>
#include <QtGlobal>
#include <cstdint>
#include <QRegularExpression>

QStringList ignoredPaths;

using namespace releaseanalyzer;

void RADebug (QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile file ("debug.txt");
    if (file.open (QIODevice::WriteOnly | QIODevice::Append))
    {
        QByteArray localMsg = msg.toLocal8Bit();
        QTextStream stream (&file);
        switch (type)
        {
            case QtDebugMsg:
                stream << localMsg.constData() << "\r\n";
                break;
            case QtWarningMsg:
                stream << "Warning:  " << localMsg.constData() << "\r\n";
                break;
            case QtFatalMsg:
                stream << "Fatal:  " << localMsg.constData() << "\r\n";
            default:
                stream << localMsg.constData() << "\r\n";
                break;
        }
        file.flush();
        file.close();
    }
}

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);
    //qInstallMessageHandler(RADebug);

    LogSettings::getInstance()->setLogFile ("ReleaseAnalyzer_LOG.txt");

    // Create PackageHandler to handle the commandline arguments concerning the packages and to
    PackageHandler packageHandler;

    return (int) packageHandler.runValidation (argc, argv);
}