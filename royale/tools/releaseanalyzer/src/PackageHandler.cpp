/***************************************************************************
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
**  -differenceFile DifferenceFileName
**  -debug
**/


/* Example Calls
*
*   releaseanalyzer.exe
*     -package "C:/Festplatte_D/DEV Tests/libroyale_0.8.5/libroyale-0.8.5-ANDROID-32Bit.tar.gz"
*     -unpackpath "C:/Festplatte_D/DEV Tests/libroyale_0.8.5/libroyale-0.8.5-ANDROID-32Bit"
*     -checkPath "C:/Festplatte_D/DEV Tests/libroyale_0.8.5/libroyale-0.8.5-ANDROID-32Bit/libroyale-0.8.5-ANDROID-32Bit"
*     -ignorepathandsubs doc
*     -validationFileName expectedStructureANDROID32.txt
*     -differenceFile DifferenceFileName
*     -debug
*
*
*   releaseanalyzer.exe
*     -package "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\libroyale-0.8.5-WINDOWS-64Bit.exe"
*     -unpackpath "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\Test"
*     -checkPath "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\Test"
*     -7zPath "C:\Program Files\7-Zip\7z.exe"
*     -ignorepathandsubs doc
*     -validationFileName expectedStructureWINDOWS64.txt
*     -differenceFile DifferenceFileName
*     -debug
*
*
*   releaseanalyzer.exe
*     -package "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\libroyale-0.8.5-WINDOWS-64Bit.exe"
*     -unpackpath "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\Test"
*     -checkPath "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\Test"
*     -7zPath "C:\Program Files\7-Zip\7z.exe"
*     -debug
*
*
*   releaseanalyzer.exe
*     -createReference "C:\Festplatte_D\DEV Tests\libroyale_0.8.5\libroyale-0.8.5-ANDROID-32Bit\libroyale-0.8.5-ANDROID-32Bit" expectedStructureANDROID32.txt
*
*/

#include <PackageHandler.hpp>
#include <CompileHandler.hpp>
#include <Logger.hpp>
#include <QThread>

#include <QCoreApplication>
#include <QRegularExpression>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlStreamReader>

#ifndef Q_OS_WIN32
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>
#endif

#define PATH_SEPARATOR QChar('/') //QDir::separator()
#define OUTPUT_HELPLINE(command, justifier, info) qDebug() << QString(command).leftJustified(45, justifier) + " " + info;

using namespace releaseanalyzer;

QProcess bddProcess;

const char *PackageHandler::controlFlowMessages[] =
{
    "Unzip",
    "Parsed and generated structure file (holding the current folder/file structure on disk)",
    "Verify parsed structure against the expectation",
    "Configuring Samples via CMake",
    "Building Samples via CMake",
    "Copying Libraries from delivery package to BDD for linking",
    "Configuring BDD via CMake",
    "Building BDD via CMake",
    "Started BDD",
    "Process for CuCumber executed"
};

PackageHandler::PackageHandler (QObject *parent) :
    QObject (parent),
    m_mode (0),
    m_workflow (0),
    m_error (0),
    m_output (0),
    m_withoutRootDirName (true),
    m_dryRun (false),
    m_generateReferenceFiles (false),
    m_debugON (false),
    m_fullAutomatic (true)
{

}

PackageHandler::~PackageHandler() { }

uint8_t inline PackageHandler::getMSB (uint32_t number)
{
    volatile uint8_t msb = 0;

    while (number >>= 1)
    {
        msb++;
    }
    //if (msb != 0) msb++;        // trick
    return msb;
}

uint32_t PackageHandler::runValidation (int argc, char *argv[])
{
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (QString (argv[i]) == "-help")
            {
                qDebug() << endl;
                qDebug() << " ***************************************************************************";
                qDebug() << " **                                                                       **";
                qDebug() << " **  Copyright(C) 2015 Infineon Technologies                              **";
                qDebug() << " **  All rights reserved.                                                 **";
                qDebug() << " **                                                                       **";
                qDebug() << " ***************************************************************************";
                qDebug() << endl;
                OUTPUT_HELPLINE ("-settingsFile <filename> ", '.', "Can be used to provide an XML Settings file");
                OUTPUT_HELPLINE ("-createReference <unpack-path> <filename> ", '.', "creates a reference file from the given directory which has to be used for -validationFileName");
                OUTPUT_HELPLINE ("-7zPath <path>", '.', "specifies the path to seven zip to to manage the control flow via 7zip (needed in case of NSIS)");
                OUTPUT_HELPLINE ("-package <package-path> ", '.', "specify the complete path to the package which shall be extracted");
                OUTPUT_HELPLINE ("-unpackpath <unpack-path> ", '.', "specify the path to which the package (-package) shall be extracted");
                OUTPUT_HELPLINE ("-checkPath <check-path> ", '.', "specify the directory tree that shall be parsed for comparing with the expectation file; might differ from -unpackpath");

                OUTPUT_HELPLINE ("-compileSamples ", '.', "specify if the samples of the pmd package shall be configured and compiled and checked");
                OUTPUT_HELPLINE ("-compileBDD ", '.', "specify if BDD shall be configured and compiled and linked against the delivered library of pmd");
                OUTPUT_HELPLINE ("-runBDD ", '.', "specify if the BDD tests shall be run automatically after compiling and linking against the pmd delivered library");

                OUTPUT_HELPLINE ("-ignorepathandsubs <path> ", '.', "adds a path to the ignorelist for checking completeness (might be given multiple times); doc");
                OUTPUT_HELPLINE ("", ' ', "directory might be a case; given directories are ignored during creation of referenceFile");
                OUTPUT_HELPLINE ("-validationFileName <fileName> ", '.', "specify the filename which shall be used to ensure completeness after unpacking");
                OUTPUT_HELPLINE ("-differenceFile <fileName> ", '.', "specifies the filename the unexpected differences between the file structure and the expectation file");
                OUTPUT_HELPLINE ("-debug ", '.', "turn debug switch on; this is for human execution - automatic execution must not use that feature");
                OUTPUT_HELPLINE ("-fullautomatic ", '.', "specify a full automatic run; useful if you already plugged in a camera otherwise a notification might come in handy");
                OUTPUT_HELPLINE ("-usecreateconfig <config> ", '.', "Reads specific config from settingsfile, useful to generate expectationfiles");

                qDebug() << endl;
                qDebug() << "Additional Information:";
                OUTPUT_HELPLINE ("[ ]", '.', "means this workflow task was planned to be executed, but for any unknown reason it wasn't");
                OUTPUT_HELPLINE ("[X]", '.', "means this workflow task was executed successfully");
                OUTPUT_HELPLINE ("[I]", '.', "Information for the user to keep track of the program execution path");
                OUTPUT_HELPLINE ("[W]", '.', "means there is a warning; something the user should know about");
                OUTPUT_HELPLINE ("[E]", '.', "means this workflow task caused any unknown failure");
                OUTPUT_HELPLINE ("[~]", '.', "means the workflow caused a serious error; any state was entered unplanned!");

                // was help command; exiting
                return 0;
            }


            if (QString (argv[i]) == "-settingsFile")
            {
                if (argc - 1 > i)
                {
                    m_settingsFileName = QString (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -settingsFile needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-createReference")
            {
                if (argc - 1 > i + 1) // read two parameters
                {
                    m_referenceStructurePath = QDir::toNativeSeparators (argv[++i]);
                    m_referenceFileName = QString (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -reference needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-debug")
            {
                m_debugON = true;
            }
            else if (QString (argv[i]) == "-7zPath")
            {
                if (argc - 1 > i)
                {
                    m_sevenZipPath = QDir::toNativeSeparators (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -7zPath needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-differenceFile")
            {
                if (argc - 1 > i)
                {
                    m_differenceFile = QDir::toNativeSeparators (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -differenceFile needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-package")
            {
                if (argc - 1 > i)
                {
                    m_packagePath = QDir::toNativeSeparators (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -package needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-unpackpath")
            {
                if (argc - 1 > i)
                {
                    m_unpackPath = QDir::toNativeSeparators (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -unpackpath needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-checkPath")
            {
                if (argc - 1 > i)
                {
                    m_checkPath = QDir::toNativeSeparators (argv[++i]);
                    //m_mode |= (uint8_t)CheckControlFlow::CheckStructureAfterParsing;
                }
                else
                {
                    qDebug() << "Argument missing: -checkPath needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-ignorepathandsubs")
            {
                if (argc - 1 > i)
                {
                    QString usedpath;
                    if (argv[++i][0] == '/')
                    {
                        usedpath = &argv[i][1];
                    }
                    else
                    {
                        usedpath = argv[i];
                    }

                    m_ignoredPaths.push_back (usedpath);
                }
                else
                {
                    qDebug() << "Argument missing: -ignorePathAndSubs needs a path";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-validationFileName")
            {
                if (argc - 1 > i)
                {
                    m_validationFileName = QString (argv[++i]);
                    m_mode |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
                }
                else
                {
                    qDebug() << "Argument missing: -validationFileName expects a file";
                    return 1;
                }
            }
            else if (QString (argv[i]) == "-compileSamples")
            {
                m_mode |= (1 << ( (uint8_t) CheckControlFlow::CompileSamples));
            }
            else if (QString (argv[i]) == "-compileBDD")
            {
                m_mode |= (1 << ( (uint8_t) CheckControlFlow::CompileBDD));
            }
            else if (QString (argv[i]) == "-runBDD")
            {
                m_mode |= (1 << ( (uint8_t) CheckControlFlow::RunBDD));
            }
            else if (QString (argv[i]) == "-fullautomatic")
            {
                m_fullAutomatic = true;
            }
            else if (QString (argv[i]) == "-usecreateconfig")
            {
                if (argc - 1 > i)
                {
                    m_useCreateConfig = QString (argv[++i]);
                }
                else
                {
                    qDebug() << "Argument missing: -useCreateConfig expects a config name";
                    return 1;
                }
            }
        }

        if (!m_settingsFileName.isEmpty())
        {
            return parseSettingsFile();
        }

        else   // Going via single command-line shot
        {
            if (!checkArgumentCombination())
            {
                qDebug() << "COMBINATION ERROR: Combination not allowed - safely exiting program";
                return 1;
            }

            validatePackage();
            return m_error;
        }
    }

    qDebug() << "ARGUMENT ERROR: Insufficient arguments - safely exiting program.";
    return 1;
}

QString PackageHandler::getCompileTimePlatform()
{
#if defined(Q_OS_ANDROID)
    if (sizeof (intptr_t) == 4) // 32 Bit
    {
        return "ANDROID-32Bit";
    }
    else if (sizeof (intptr_t) == 8) // 64 Bit
    {
        return "ANDROID-64Bit";
    }
    else
    {
        return "ANDROID";
    }
#elif defined(Q_OS_IOS) || defined(Q_OS_MAC) || defined(Q_OS_DARWIN) ||  defined(Q_OS_OSX)
    if (sizeof (intptr_t) == 4) // 32 Bit
    {
        return "APPLE-32Bit";
    }
    else if (sizeof (intptr_t) == 8) // 64 Bit
    {
        return "APPLE-64Bit";
    }
    else
    {
        return "APPLE";
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX) || defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_FREEBSD) || defined(Q_OS_CYGWIN)
    if (sizeof (intptr_t) == 4) // 32 Bit
    {
        return "LINUX-32Bit";
    }
    else if (sizeof (intptr_t) == 8) // 64 Bit
    {
        return "LINUX-64Bit";
    }
    else
    {
        return "LINUX";
    }
#elif defined(Q_OS_WIN)
#if defined(Q_OS_WIN64)
    return "WINDOWS-64Bit";
#elif defined(Q_OS_WIN32)
    return "WINDOWS-32Bit";
#else
    return "WINDOWS";
#endif
#endif
}

QString PackageHandler::getPlatformType()
{
    QProcess currentPlatform;
    currentPlatform.start ("uname -a");

    // Wait till configuration is started
    currentPlatform.waitForStarted (-1);
    // Wait till configuration is finished
    currentPlatform.waitForFinished (-1);

    QString result = QString (currentPlatform.readAllStandardOutput());
    //qDebug() << result;

    if (QRegularExpression ("Linux").match (result).hasMatch())        // Linux
    {
        if (sizeof (intptr_t) == 4) // 32 Bit
        {
            return "LINUX-32Bit";
        }
        else if (sizeof (intptr_t) == 8) // 64 Bit
        {
            return "LINUX-64Bit";
        }
        else
        {
            return "LINUX";
        }
    }
    else if (QRegularExpression ("Unix").match (result).hasMatch())    // Unix
    {
        if (sizeof (intptr_t) == 4) // 32 Bit
        {
            return "LINUX-32Bit";
        }
        else if (sizeof (intptr_t) == 8) // 64 Bit
        {
            return "LINUX-64Bit";
        }
        else
        {
            return "LINUX";
        }
    }
    else if (QRegularExpression ("Darwin").match (result).hasMatch())  // Apple
    {
        if (sizeof (intptr_t) == 4) // 32 Bit
        {
            return "APPLE-32Bit";
        }
        else if (sizeof (intptr_t) == 8) // 64 Bit
        {
            return "APPLE-64Bit";
        }
        else
        {
            return "APPLE";
        }
    }
    else if (result.isEmpty())
    {
        if (sizeof (intptr_t) == 4) // 32 Bit
        {
            return "WINDOWS-32Bit";
        }
        else if (sizeof (intptr_t) == 8) // 64 Bit
        {
            return "WINDOWS-64Bit";
        }
        else
        {
            return "WINDOWS";
        }
    }

    // might be android!?!?
    if (sizeof (intptr_t) == 4) // 32 Bit
    {
        return "ANDROID-32Bit";
    }
    else if (sizeof (intptr_t) == 8) // 64 Bit
    {
        return "ANDROID-64Bit";
    }

    return "ANDROID";
}

uint16_t PackageHandler::parseSettingsFile()
{
    uint16_t globError = 0;
    QString platformType = PackageHandler::getCompileTimePlatform();

    if (!m_useCreateConfig.isEmpty())
    {
        platformType = m_useCreateConfig;
    }

    DLOG (INFO) << "Platform/OS: " << platformType;

    QFile settingsFile (m_settingsFileName);
    settingsFile.open (QIODevice::ReadOnly);
    const QByteArray settingsDataFile = settingsFile.readAll();

    QFile schemaFileSettings (QStringLiteral ("settings.xsd"));
    schemaFileSettings.open (QIODevice::ReadOnly);
    const QByteArray schemaDataSettings = schemaFileSettings.readAll();

    QXmlSchema m_xmlSchemaSettings;
    m_xmlSchemaSettings.load (schemaDataSettings);

    QXmlSchemaValidator validator (m_xmlSchemaSettings);
    bool useCaseLoaded = validator.validate (settingsDataFile);
    if (!useCaseLoaded)
    {
        DLOG (WARN) << "ERROR: The passed Settingsfile had an incorrect Fileformat.";
    }

    QXmlStreamReader xs (settingsDataFile);
    QString curPlatform;
    while (!xs.atEnd())
    {
        if (xs.readNextStartElement())
        {
            if ("Settings" == xs.name())
            {
                xs.readNextStartElement();
            }

            if ("Platform" == xs.name())
            {
                curPlatform = xs.attributes().at (0).value().toString();
                if (curPlatform.contains (platformType))
                {
                    DLOG (INFO) << "[I]" << "----------------------------- Platform: " + curPlatform.append (' ').leftJustified (40, '-');
                }
                xs.readNextStartElement();
            }

            if ("Package" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_packagePath = QDir::toNativeSeparators (xs.readElementText());
                }
                xs.readNextStartElement();
            }

            if ("Unpackpath" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_unpackPath = QDir::toNativeSeparators (xs.readElementText());
                }
                xs.readNextStartElement();
            }

            if ("Checkpath" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_checkPath = QDir::toNativeSeparators (xs.readElementText());
                }
                xs.readNextStartElement();
            }

            if ("SevenZipPath" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_sevenZipPath = QDir::toNativeSeparators (xs.readElementText());
                }
                xs.readNextStartElement();
            }

            if ("Ignorepathandsubs" == xs.name())
            {
                while (xs.readNextStartElement() && "Path" == xs.name())
                {
                    if (curPlatform.contains (platformType))
                    {
                        QString path = xs.readElementText();
                        m_ignoredPaths.push_back (path.constData() [0] == '/' ? path.mid (1) : path);
                    }
                }
                xs.readNextStartElement();
            }

            if ("Validationfile" == xs.name())
            {
                if (curPlatform.contains (platformType) && m_useCreateConfig.isEmpty())
                {
                    m_mode |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
                    m_validationFileName = QDir::toNativeSeparators (xs.readElementText());
                }

                else if (!m_useCreateConfig.isEmpty())
                {
                    m_referenceFileName = QDir::toNativeSeparators (xs.readElementText());
                    //m_referenceStructurePath = QDir::toNativeSeparators (m_referenceFileName);
                }

                xs.readNextStartElement();
            }

            if ("Differencefile" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_differenceFile = QDir::toNativeSeparators (xs.readElementText());
                }
                xs.readNextStartElement();
            }

            if ("Debug" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_debugON = (xs.readElementText().toInt() >= 1 ? true : false);
                }
                xs.readNextStartElement();
            }

            if ("CompileSamples" == xs.name())
            {
                int value = xs.readElementText().toInt();
                if (curPlatform.contains (platformType) && m_useCreateConfig.isEmpty())
                {
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::ConfigureSamples))) : m_mode);
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::CompileSamples))) : m_mode);
                }

                xs.readNextStartElement();
            }

            if ("CompileBDD" == xs.name())
            {
                int value = xs.readElementText().toInt();
                if (curPlatform.contains (platformType) && m_useCreateConfig.isEmpty())
                {
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::CopyLibs))) : m_mode);
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::ConfigureBDD))) : m_mode);
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::CompileBDD))) : m_mode);
                }

                xs.readNextStartElement();
            }

            if ("RunBDD" == xs.name())
            {
                int value = xs.readElementText().toInt();
                if (curPlatform.contains (platformType) && m_useCreateConfig.isEmpty())
                {
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::RunBDD))) : m_mode);
                    m_mode = (value >= 1 ? (m_mode | (1 << ( (uint8_t) CheckControlFlow::RunCucumber))) : m_mode);
                }
                xs.readNextStartElement();
            }

            if ("FullAutomatic" == xs.name())
            {
                if (curPlatform.contains (platformType))
                {
                    m_fullAutomatic = (xs.readElementText().toInt() >= 1 ? true : false);
                }
                xs.readNextStartElement();
            }

            if (curPlatform.contains (platformType))
            {
                if (checkArgumentCombination())
                {
                    validatePackage();

                    if (m_error == 0)       // no errors yet, continue building
                    {
                        compileAgainstDeliveredLib();
                        runBDD();           // Spawns a thread for keeping BDD running while we execute cucumber later on
                        // The process normally ends if QProcess goes out of scope, this is to avoid ending the process too early.
                        QThread::sleep (2); // Wait till BDD is running
                        //while (bddThread)
                        //{
                        //QThread::sleep (2);
                        //bddThread->kill();
                        //bddThread->wait();
                        //}
                    }
                    else
                    {
                        DLOG (ERROR) << "[E]" << "----------------------------- EOF ERROR REPORT -----------------------------";
                    }

                    globError += ( (m_error > 0) ? 1 : 0);

                    DLOG (INFO) << "[I]" << "----------------------------- SUMMARY REPORT -----------------------------";
                    m_output = 0;
                    printControlFlow();
                    DLOG (INFO) << "[I]" << "--------------------------- EOF SUMMARY REPORT ---------------------------";

                    /* Reset Statemachine variables for next potential platform */
                    m_mode = 0;
                    m_workflow = 0;
                    m_output = 0;
                    m_error = 0;
                    m_expectedVector.clear();
                    m_parsedVector.clear();
                    m_errorMessage.clear();

                    m_referenceStructurePath.clear();
                    m_referenceFileName.clear();
                    m_unpackPath.clear();
                    m_sevenZipPath.clear();
                    m_checkPath.clear();
                    m_packagePath.clear();
                    m_validationFileName.clear();
                    m_differenceFile.clear();
                    m_ignoredPaths.clear();
                }
                else
                {
                    DLOG (ERROR) << "COMBINATION ERROR: Combination not allowed - safely exiting program";
                    return 1;
                }
            }
            // Nothing to do here
        }
        // Nothing to do here; just no element left to be handled
    }

    return globError;
}

bool PackageHandler::checkArgumentCombination()
{
    bool hasZippedPackage = !m_packagePath.isEmpty() && !m_unpackPath.isEmpty();

    if ( (m_packagePath.isEmpty()) ^ (m_unpackPath.isEmpty()))
    {
        DLOG (ERROR) << "ERROR: If PackagePath is Given, then unpackpath has to be given as well!";
        return false;
    }

    if (hasZippedPackage && !m_referenceStructurePath.isEmpty())
    {
        DLOG (ERROR) << "ERROR: Ambiguous information given, you cannot provide a path together with a package- and unpackpath!";
        return false;
    }

    if (!hasZippedPackage && m_referenceStructurePath.isEmpty())
    {
        DLOG (ERROR) << "ERROR: You have to specify an unzipped directory path or zip- and unzip paths!";
        return false;
    }

    if (hasZippedPackage)
    {
        m_mode |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
    }

    m_mode |= (1 << ( (uint8_t) CheckControlFlow::ParseDir));
    return true;
}

void PackageHandler::setDryRun (bool dryRun)
{
    m_dryRun = dryRun;
}

bool PackageHandler::compareFiles (QString validationFileName, QString validationFilePath, QString parsedStructureFileName, QString parsedStructureFilePath)
{
    bool identical = true;

    QFile file1Handle (validationFilePath);
    if (!file1Handle.open (QIODevice::ReadOnly | QIODevice::Text))
    {
        m_errorMessage = "ERROR opening " + validationFileName + " in compareFiles()";
        m_error |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
        return identical;
    }

    QFile file2Handle (parsedStructureFilePath);
    if (!file2Handle.open (QIODevice::ReadOnly | QIODevice::Text))
    {
        m_errorMessage = "ERROR opening " + parsedStructureFileName + " in compareFiles()";
        m_error |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
        return identical;
    }

    QTextStream file1Stream (&file1Handle);
    QTextStream file2Stream (&file2Handle);

    QString file1Line;
    QString file2Line;

    bool ignoreFile1temp = true;
    bool ignoreFile2temp = true;
    do
    {
        if (ignoreFile1temp && ignoreFile2temp)
        {
            file1Line = file1Stream.readLine();
            file2Line = file2Stream.readLine();
        }
        else
        {
            if (ignoreFile1temp == false)
            {
                file1Line = file1Stream.readLine();
            }

            if (ignoreFile2temp == false)
            {
                file2Line = file2Stream.readLine();
            }
        }

        ignoreFile1temp = true;
        ignoreFile2temp = true;

        for (uint8_t i = 0; i < m_ignoredPaths.count(); ++i)
        {
            QString pattern ("^.*[/]" + m_ignoredPaths.at (i) + ".*");
            if (file1Line.contains (QRegularExpression (pattern, QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption)))
            {
                ignoreFile1temp = false;
            }

            if (file2Line.contains (QRegularExpression (pattern, QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption)))
            {
                ignoreFile2temp = false;
            }
        }

        if (ignoreFile1temp && ignoreFile2temp)
        {
            if (file1Line != file2Line)
            {



                if (file1Line.isEmpty())
                {
                    //m_errorMessage = "EOF reached for: " + validationFileName;
                    //m_error |= (1 << (getMSB(m_workflow) + 1));
                    break;
                }

                else if (file2Line.isEmpty())
                {
                    //qCritical() << "EOF reached: " << file2Name;
                    m_errorMessage = "EOF reached for: " + parsedStructureFileName;
                    m_error |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
                }

                else
                {
                    identical = false;
                    ignoreFile2temp = false;
                }
            }
            else
            {
                if (identical == false)
                {
                    identical = true;
                }
            }
        }
    }
    while (!file2Line.isNull() || !file1Line.isNull());

    if (!identical)
    {
        m_errorMessage = "Files are not identical: " + validationFileName + " and " + parsedStructureFileName;
        m_error |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
    }

    return identical;
}

bool PackageHandler::compareVectors()
{
    bool identical = true;

    for (auto it = m_expectedVector.begin(); it != m_expectedVector.end(); ++it)
    {
        bool cont = false;
        for (uint8_t i = 0; i < m_ignoredPaths.count(); ++i)
        {
            QString pattern ("^.*[/]*" + m_ignoredPaths.at (i) + ".*$");
            if ( (*it).contains (QRegularExpression (pattern, QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption)))
            {
                cont = true;
                break;
            }
        }

        int indexOfElement = m_parsedVector.indexOf (*it);
        if (indexOfElement >= 0)
        {
            if (cont == false)
            {
                m_parsedVector.removeAt (indexOfElement);
            }

            else if (cont == true)
            {
                m_parsedVector.removeAt (indexOfElement);
                continue;
            }
        }

        else if (cont == false)
        {
            if (m_debugON)
            {
                LOG (ERROR) << "[E]" << "Expected File not found: " << *it;
            }
            m_errorMessage = "ERROR: one or more expected files are not included in the given package";
            m_error |= (1 << (uint8_t) CheckControlFlow::VerifyWithFile);
            identical = false;
        }
    }

    if (!m_parsedVector.isEmpty())
    {
        QFile *file1Handle = nullptr;
        if (!m_differenceFile.isEmpty())
        {
            file1Handle = new QFile (m_differenceFile);
            if (!file1Handle->open (QIODevice::WriteOnly | QIODevice::Text))
            {
                m_errorMessage = "ERROR opening " + m_differenceFile;
                m_error |= (1 << (uint8_t) CheckControlFlow::VerifyWithFile);
            }
        }

        for (auto it = m_parsedVector.begin(); it != m_parsedVector.end(); ++it)
        {
            if (file1Handle)
            {
                QTextStream (file1Handle) << "Parsing revealed unexpected file: " << *it << endl;
            }
            if (m_debugON)
            {
                DLOG (WARN) << "[W]+ " << "Parsing revealed unexpected file: " << *it;
            }
        }
        if (file1Handle != nullptr)
        {
            file1Handle->close();
            delete file1Handle;
        }
    }
    return identical;
}

uint32_t PackageHandler::validatePackage()
{
    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::Unzip)))
    {
        //  Unzip Archive
        if (!m_packagePath.isEmpty())
        {
            // Unzipping here
            unzipPackage();
            m_workflow |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
        }
    }

    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::ParseDir)))
    {
        // Now parse the dir recursively
        if (m_checkPath.isEmpty())
        {
            // Take normal inpath, there was no archive!
            parseDir (m_referenceStructurePath);
        }
        else
        {
            // got a tar package .. trying untar!
            parseDir (m_checkPath);
        }

        m_workflow |= (1 << ( (uint8_t) CheckControlFlow::ParseDir));
    }

    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile)))
    {
        QFile fileHandle (m_validationFileName);
        if (!fileHandle.open (QIODevice::ReadOnly | QIODevice::Text))
        {
            m_errorMessage = "ERROR opening " + m_validationFileName;
            m_error |= (1 << (uint8_t) CheckControlFlow::VerifyWithFile);
        }

        QTextStream fileStream (&fileHandle);

        QString fileLine;
        do
        {
            fileLine = fileStream.readLine();
            if (!fileLine.isEmpty())
            {
                m_expectedVector << fileLine;
            }
        }
        while (!fileLine.isNull());

        compareVectors();

        // Verify generated File with expectationsfile, by ignoring certain dirs.
        m_workflow |= (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile));
    }

    printControlFlow();  // providing control Flow

    return m_error;
}

uint32_t PackageHandler::compileAgainstDeliveredLib()
{
    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::ConfigureSamples)))
    {
        if (!m_checkPath.isEmpty())
        {
            QDir srcPath (qApp->applicationDirPath()); // should be projects root path
            srcPath.cdUp();
            srcPath.cdUp();
            srcPath.cdUp();

            CompileHandler compileHandler (m_checkPath, srcPath.absolutePath());
            compileHandler.setDebug (true);

            if (compileHandler.configureSamples())
            {
                m_workflow |= (1 << ( (uint8_t) CheckControlFlow::ConfigureSamples));
            }
            else
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::ConfigureSamples));
            }
        }
        else
        {
            m_errorMessage = "ERROR: Checkpath is missing - should be provided in the config or via -checkPath";
            m_error |= (1 << ( (uint8_t) CheckControlFlow::ConfigureSamples));
        }
    }

    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::CompileSamples)))
    {
        if (!m_checkPath.isEmpty())
        {
            QDir srcPath (qApp->applicationDirPath()); // should be projects root path
            srcPath.cdUp();
            srcPath.cdUp();
            srcPath.cdUp();

            CompileHandler compileHandler (m_checkPath, srcPath.absolutePath());
            compileHandler.setDebug (true);

            if (compileHandler.buildSamples())
            {
                m_workflow |= (1 << ( (uint8_t) CheckControlFlow::CompileSamples));
            }
            else
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::CompileSamples));
            }
        }
        else
        {
            m_errorMessage = "ERROR: Checkpath is missing - should be provided in the config or via -checkPath";
            m_error |= (1 << ( (uint8_t) CheckControlFlow::CompileSamples));
        }
    }

    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::CopyLibs)))
    {
        if (!m_checkPath.isEmpty())
        {
            // samples could be compiled; lets copy the library to build BDD tests from source against this one!
            QDir findRoyaleLib (m_checkPath);

#if defined(Q_OS_WIN)
            /*findRoyaleLib.cd("lib");
            if(QFile::exists(findRoyaleLib.absoluteFilePath("royale.lib")))
            {
                QDir copyDir(qApp->applicationDirPath());
                copyDir.cdUp();
                copyDir.cdUp();
                copyDir.cd("bdd");
                copyDir.mkdir("lib");
                copyDir.cd("lib");

                QFile::copy(findRoyaleLib.absoluteFilePath("royale.lib"), copyDir.absoluteFilePath("royale.lib"));
                DLOG(INFO) << "[I] Copied royale.lib";
            }*/

            //findRoyaleLib.cdUp();
            findRoyaleLib.cd ("bin");
            if (QFile::exists (findRoyaleLib.absoluteFilePath ("royale.dll")))
            {
                QDir copyDir (qApp->applicationDirPath());

                QFile::copy (findRoyaleLib.absoluteFilePath ("royale.dll"), copyDir.absoluteFilePath ("royale.dll"));
                DLOG (INFO) << "[I] Copied royale.dll";
            }
#else
            QStringList fileFilter;
#if defined(Q_OS_IOS) || defined(Q_OS_MAC) || defined(Q_OS_DARWIN) ||  defined(Q_OS_OSX)
            fileFilter << "*.dylib";
#else
            fileFilter << "*.so*";
#endif

            findRoyaleLib.cd ("bin");
            QStringList libraryFileList = findRoyaleLib.entryList (fileFilter); // copy symlinks as well; necessary to get it working on MAC

            QDir bddExecutionFolder (qApp->applicationDirPath());
            /*QDir bddLibFolder(qApp->applicationDirPath());
            bddLibFolder.cdUp();
            bddLibFolder.cdUp();
            bddLibFolder.cd("bdd");
            bddLibFolder.mkdir("lib");
            bddLibFolder.cd("lib");    // reached BDD Lib Folder
            */

            for (auto i : libraryFileList)
            {
                //QFile::copy(findRoyaleLib.absoluteFilePath(i), bddLibFolder.absoluteFilePath(i));
                QFile::copy (findRoyaleLib.absoluteFilePath (i), bddExecutionFolder.absoluteFilePath (i));
                DLOG (INFO) << "[I] Copied " << i;
            }
#endif
            m_workflow |= (1 << ( (uint8_t) CheckControlFlow::CopyLibs));
        }
        else
        {
            m_errorMessage = "ERROR: Checkpath is missing - should be provided in the config or via -checkPath";
            m_error |= (1 << ( (uint8_t) CheckControlFlow::CopyLibs));
        }
    }

    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::ConfigureBDD)))
    {
        if (!m_checkPath.isEmpty())
        {
            QDir srcPath (qApp->applicationDirPath()); // should be projects root path
            srcPath.cdUp();
            srcPath.cdUp();
            srcPath.cdUp();

            CompileHandler compileHandler (m_checkPath, srcPath.absolutePath());
            compileHandler.setDebug (true);

            if (compileHandler.configureBDD())
            {
                m_workflow |= (1 << ( (uint8_t) CheckControlFlow::ConfigureBDD));
            }
            else
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::ConfigureBDD));
            }
        }
        else
        {
            m_errorMessage = "ERROR: Checkpath is missing - should be provided in the config or via -checkPath";
            m_error |= (1 << ( (uint8_t) CheckControlFlow::ConfigureBDD));
        }
    }

    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::CompileBDD)))
    {
        if (!m_checkPath.isEmpty())
        {
            QDir srcPath (qApp->applicationDirPath()); // should be projects root path
            srcPath.cdUp();
            srcPath.cdUp();
            srcPath.cdUp();

            CompileHandler compileHandler (m_checkPath, srcPath.absolutePath());
            compileHandler.setDebug (true);

            if (compileHandler.buildBDD())
            {
                m_workflow |= (1 << ( (uint8_t) CheckControlFlow::CompileBDD));
            }
            else
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::CompileBDD));
            }
        }
        else
        {
            m_errorMessage = "ERROR: Checkpath is missing - should be provided in the config or via -checkPath";
            m_error |= (1 << ( (uint8_t) CheckControlFlow::CompileBDD));
        }
    }

    printControlFlow();  // providing control Flow
    return m_error;
}

uint32_t PackageHandler::runBDD()
{
    printControlFlow();  // providing control Flow
    if (m_mode & (1 << ( (uint8_t) CheckControlFlow::RunBDD)))
    {
        // Start configuration
        connect (&bddProcess, SIGNAL (readyReadStandardOutput()), this, SLOT (readOutput (void)));
#if defined(Q_OS_WIN)
        qDebug() << "starting BDD Process";
        bddProcess.start ("test_royale_bdd.exe", QIODevice::ReadWrite | QIODevice::Text);
#else
        bddProcess.start ("./test_royale_bdd");
#endif
        QThread::sleep (2);
        qDebug() << "waiting for finishing of BDD Process...";

        if (!bddProcess.waitForFinished (-1))
        {
            qDebug() << "executing program failed with exit code" << bddProcess.exitCode();
        }
        else
        {
            qDebug() << QString (bddProcess.readAllStandardOutput());
        }
    }

    printControlFlow();  // providing control Flow
    return m_error;
}

void PackageHandler::readOutput()
{
    bddProcess.setReadChannel (QProcess::StandardOutput);
    while (bddProcess.canReadLine())
    {
        qDebug() << QString (bddProcess.readLine());
    }
}

uint8_t inline PackageHandler::dumpMissingCases (uint32_t state)
{
    uint8_t i = 0;
    while ( ( (uint32_t) (1 << i)) < state)
    {
        if (! (m_output & (1 << i)))
        {
            if ( (m_mode & (1 << i)) && ! (m_workflow & (1 << i)))
            {
                /* Being here means:
                * STATE was forseen to be executed, but it wasn't!!
                * Write out the state that was skipped/not executed for unknown reasons!!
                */
                // Check if there was an error anywhere!
                if (m_error & (1 << i))
                {
                    DLOG (ERROR) << "[E]" << controlFlowMessages[i];     // write out the state here! (we are missing that one; WORKFLOW ERROR)
                    if (!m_errorMessage.isEmpty())
                    {
                        DLOG (ERROR) << "   " << m_errorMessage;
                        m_errorMessage.clear();
                    }
                }
                else
                {
                    DLOG (ERROR) << "[ ]" << controlFlowMessages[i];     // write out the state here! (we are missing that one; WORKFLOW ERROR)
                }
                m_output |= (1 << i);
            }
        }
        i++;
    }
    return i;
}

void PackageHandler::controlFlow (const uint32_t state)
{
    if (! (m_output & state))
    {
        /* Being here means:
        * STATE was not written to the command-line as "has to be executed", "was skipped", etc...
        */
        if (m_mode & state)
        {
            /* Being here means:
            * STATE is forseen to be executed regarding to the command-line arguments */

            if (m_error & state)
            {
                // There was an error for this state!
                /* Dump older states; that might have been skipped/not executed for an unknown reason */
                uint8_t msb_no = dumpMissingCases (state);

                /* After we have written out all previously skipped states, we
                * continue writing our state out! - the current one, which was asked for by the method (input parameter) */
                DLOG (ERROR) << "[E]" << controlFlowMessages[msb_no];
                if (!m_errorMessage.isEmpty())
                {
                    DLOG (ERROR) << "   " << m_errorMessage;
                    m_errorMessage.clear();
                }
                m_output |= state;
            }

            else if (m_workflow & state)
            {
                /* Being here means:
                * STATE was already executed according to the workflow */

                /* Dump older states; that might have been skipped/not executed for an unknown reason */
                uint8_t msb_no = dumpMissingCases (state);

                /* After we have written out all previously skipped states, we
                * continue writing our state out! - the current one, which was asked for by the method (input parameter) */
                DLOG (INFO) << "[X]" << controlFlowMessages[msb_no];
                m_output |= state;
            }

            /* STATE not in workflow yet; will possibly be executed later - be patient
            * States are looped...
            */
        }
        else if (! (m_mode & state) && (m_workflow & state)) // STATE in Workflow where it shouldn't be! Serious ERROR
        {
            /* Dump all states that were not dumped yet and were part of the workflow and end program */
            uint8_t msb_no = dumpMissingCases (state);
            DLOG (ERROR) << "[~]" << controlFlowMessages[msb_no];
            qCritical() << "UNKNOWN STATE TRANSITION ERROR: State entered unplanned!";
        }
    }
}

void PackageHandler::printControlFlow()
{
    if (m_debugON)
    {
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::Unzip)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::ParseDir)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::VerifyWithFile)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::ConfigureSamples)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::CompileSamples)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::ConfigureBDD)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::CompileBDD)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::RunBDD)));
        controlFlow ( (1 << ( (uint8_t) CheckControlFlow::RunCucumber)));
    }
    else
    {
        if ( (m_workflow + m_error) == m_mode && m_output != m_mode)
        {
            // all states covered anyhow; does not matter if executed was ok
            m_output = m_mode;
            qDebug() << m_error;
            exit (m_error);
        }
    }
}

uint32_t PackageHandler::getControlFlow()
{
    return m_mode;
}

void PackageHandler::unzipPackage()
{
    if (!QFile::exists (m_packagePath))
    {
        m_errorMessage = "ERROR: given package file not existing at path: " + m_packagePath;
        m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
        return;
    }

#ifdef Q_OS_WIN
    if (!m_sevenZipPath.isEmpty())
    {
        QProcess sevenZip;
        sevenZip.start (m_sevenZipPath, QStringList() << "x" << "-o" + m_unpackPath << m_packagePath);

        sevenZip.waitForFinished();
        return;
    }
#else
    {
        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int flags;
        int r;

        /* Select which attributes we want to restore. */
        flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        // Initialize writer (decompressed archive structure destination)
        ext = archive_write_disk_new();
        archive_write_disk_set_options (ext, flags);
        archive_write_disk_set_standard_lookup (ext);

        // Initialize reader (compressed archive)
        char compressedArchive[300];
        memset (&compressedArchive, 0, sizeof (compressedArchive));
        memcpy (compressedArchive, m_packagePath.toStdString().c_str(), sizeof (compressedArchive));

        a = archive_read_new();
        archive_read_support_format_all (a);
        archive_read_support_compression_all (a);
        r = archive_read_open_filename (a, compressedArchive, 10240);
        if (r != ARCHIVE_OK)
        {
            m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
            m_errorMessage = archive_error_string (a);
            printControlFlow();
            exit (1);
        }
        while (1)
        {
            r = archive_read_next_header (a, &entry);
            if (r == ARCHIVE_EOF)
            {
                break;
            }
            if (r < ARCHIVE_OK)
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                m_errorMessage = archive_error_string (a);
                //fprintf(stderr, "%s\n", archive_error_string(a));
            }
            if (r < ARCHIVE_WARN)
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                m_errorMessage = archive_error_string (a);
                printControlFlow();
                exit (1);
            }

            /**
             * UGLY! Necessary hack to make extractor write to given output directory!
             */
            QString oldPathqt (archive_entry_pathname (entry));     // We need the old Path for later use (checking unzip via file)
            QString newPathqt (m_unpackPath + QDir::separator() + archive_entry_pathname (entry));

            char newPath[1000 + 1];
            strncpy (newPath, newPathqt.toStdString().c_str(), sizeof (newPath));
            archive_entry_set_pathname (entry, newPath);            // change pathname so libarchive writes to different directory!
            /* *************************** END OF HACK ******************************* */

            r = archive_write_header (ext, entry);

            if (r < ARCHIVE_OK)
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                m_errorMessage = archive_error_string (ext);
                //fprintf(stderr, "%s\n", archive_error_string(ext));
            }
            else if (archive_entry_size (entry) > 0)
            {


                //r = copy_data(a, ext); -- could be an external function; we use it inline!
                // ext --> this it the "ghost" archive we are writing to; normal HDD stream
                // a   --> this is the real archive we are reading from
                const void *buff;
                size_t size;
                la_int64_t offset;
                for (int ir = 0;;)
                {
                    ir = archive_read_data_block (a, &buff, &size, &offset);

                    if (ir == ARCHIVE_EOF)
                    {
                        r = (ARCHIVE_OK);
                        break;
                    }
                    if (ir < ARCHIVE_OK)
                    {
                        r = (ir);
                        break;
                    }

                    ir = archive_write_data_block (ext, buff, size, offset);
                    if (ir < ARCHIVE_OK)
                    {
                        r = (ir);
                        break;
                    }

                    //archive_entry_set_pathname(entry, oldPath);
                }

                if (r < ARCHIVE_OK)
                {
                    m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                    m_errorMessage = archive_error_string (ext);
                    //fprintf(stderr, "%s\n", archive_error_string(ext));
                }

                if (r < ARCHIVE_WARN)
                {
                    m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                    m_errorMessage = archive_error_string (ext);
                    printControlFlow();
                    exit (1);
                }


            }
            r = archive_write_finish_entry (ext);

            if (r < ARCHIVE_OK)
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                m_errorMessage = archive_error_string (ext);
                //fprintf(stderr, "%s\n", archive_error_string(ext));
            }
            if (r < ARCHIVE_WARN)
            {
                m_error |= (1 << ( (uint8_t) CheckControlFlow::Unzip));
                m_errorMessage = archive_error_string (ext);
                printControlFlow();
                exit (1);
            }
        }
        archive_read_close (a);
        archive_read_free (a);
        archive_write_close (ext);
        archive_write_free (ext);
    }
#endif
}

void PackageHandler::parseDir (QString curDirPath)
{
    QLocale::setDefault (QLocale (QLocale::Finnish));

    QFile file (QCoreApplication::applicationDirPath() + QDir::separator() + (m_referenceFileName.isEmpty() ? "parsedStructure.txt" : m_referenceFileName));
    if (!file.open (QIODevice::WriteOnly | QIODevice::Text))
    {
        m_errorMessage = "ERROR: Cannot create parsed structure parse file";
        m_error |= (1 << ( (uint8_t) CheckControlFlow::ParseDir));
        return;
    }

    if (!QDir (curDirPath).exists())
    {
        m_errorMessage = "ERROR: dir to parse does not exist: " + curDirPath;
        m_error |= (1 << ( (uint8_t) CheckControlFlow::ParseDir));
        return;
    }

    m_parseStream = new QTextStream (&file);

    if (m_dryRun)
    {
        DLOG (INFO) << "DRY RUN";
        *m_parseStream << "This is the dry-RUN output to File!" << endl;
        *m_parseStream << "Make sure there was a newline" << endl;
        *m_parseStream << "Parsed dir would have been: " << curDirPath << endl;
    }
    else
    {
        parseDirRecursivly (curDirPath, curDirPath);
    }

    file.close();
    delete m_parseStream;
}

void PackageHandler::parseDirRecursivly (QString curDirPath, const QString &operatingMainPath)
{
    QDir actDir (curDirPath);

    QStringList myListableObjects   = actDir.entryList ( (QDir::Dirs | QDir::Files | QDir::NoDot | QDir::NoDotDot), (QDir::Name));
    QStringList myDirObjects        = actDir.entryList ( (QDir::AllDirs | QDir::NoDot | QDir::NoDotDot));

    for (int i = 0; i < myListableObjects.count(); i++)
    {

        if (myDirObjects.contains (myListableObjects.at (i), Qt::CaseInsensitive))
        {
            // This is a directory, lets enter here
            parseDirRecursivly (curDirPath + PATH_SEPARATOR + myListableObjects.at (i), operatingMainPath);
        }

        else
        {
            // This is a File, lets list it
            const QString &dirReference = curDirPath.mid (curDirPath.indexOf (operatingMainPath) + operatingMainPath.length() + 1);
            const QString &parsingpath = dirReference + PATH_SEPARATOR + myListableObjects.at (i);
            if (dirReference.isEmpty())
            {
                *m_parseStream << myListableObjects.at (i) << endl;
                m_parsedVector << myListableObjects.at (i);
            }
            else
            {
                *m_parseStream << parsingpath << endl;
                m_parsedVector << parsingpath;
            }
        }
    }
}

#undef PATH_SEPARATOR
