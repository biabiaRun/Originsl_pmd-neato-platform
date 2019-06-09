/***************************************************************************
**                                                                        **
**  Copyright (C) 2015 Infineon Technologies                              **
**  All rights reserved.                                                  **
**                                                                        **
****************************************************************************/

#include <CompileHandler.hpp>
#include <Logger.hpp>

#include <QCoreApplication>
#include <QRegularExpression>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QProcess>

using namespace releaseanalyzer;

static inline int16_t checkCompilerOutput (QString output)
{
    int16_t globErrorCount = 0;
#if defined(Q_OS_WIN)
    QRegularExpression findBuildSucceded ("Build succeeded.*(?<warningcount>\\d+)\\sWarning\\(s\\)\\s*(?<errorcount>\\d+)\\sError\\(s\\)"); // Build succeeded.\\s*(?<warningcount>\\d+)\\sWarning(s)\\s*(?<errorcount>\\d+)\\sError(s)
    findBuildSucceded.setPatternOptions (QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    if (!findBuildSucceded.isValid())
    {
        QString errorString     = findBuildSucceded.errorString();
        int errorOffset         = findBuildSucceded.patternErrorOffset();

        DLOG (ERROR) << "error in regExp: " << errorString << "at offset:" << errorOffset;
        return -1;
    }

    QRegularExpressionMatch match = findBuildSucceded.match (output);
    if (match.hasMatch())
    {
        QString warningCount    = match.captured ("warningcount");
        QString errorCount      = match.captured ("errorcount");

        globErrorCount += errorCount.toInt();
    }
    else
    {
        // Something was wrong during compilation; compiling wasn't possible - probably configure wasn't run before?
        DLOG (ERROR) << "Compilation failed due to unknown reason; make sure build was configured before!";
        return -1;
    }
#else
    QRegularExpression findBuildSucceded ("(?<errorcount>\\d+) errors generated");
    findBuildSucceded.setPatternOptions (QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    if (!findBuildSucceded.isValid())
    {
        QString errorString     = findBuildSucceded.errorString();
        int errorOffset         = findBuildSucceded.patternErrorOffset();

        DLOG (ERROR) << "error in regExp: " << errorString << "at offset:" << errorOffset;
        return -1;
    }

    QRegularExpressionMatch match = findBuildSucceded.match (output);
    if (match.hasMatch())
    {
        QString errorCount      = match.captured ("errorcount");

        globErrorCount += errorCount.toInt();
    }
#endif
    return globErrorCount;
}

CompileHandler::CompileHandler (QString installPath, QString srcPath, QObject *parent)
    : QObject (parent), m_debug (false)
{
    m_installPath = installPath;
    m_srcPath = srcPath;
}

CompileHandler::~CompileHandler() { }

void CompileHandler::setDebug (bool debug)
{
    m_debug = debug;
}

bool CompileHandler::configureBDD()
{
    QDir actDir (m_srcPath);
    if (!actDir.cd ("testing"))
    {
        return false;
    }
    if (!actDir.cd ("bdd"))
    {
        return false;
    }
    actDir.mkdir ("build");
    actDir.cd ("build");

    QDir binDir (m_srcPath);
    if (!binDir.cd ("testing"))
    {
        return false;
    }
    if (!binDir.cd ("releaseanalyzer"))
    {
        return false;
    }
    if (!binDir.cd ("bin"))
    {
        return false;
    }

    QString royaleSDK ("-DROYALE_SDK_DIR=" + qApp->applicationDirPath() + QDir::separator() + m_installPath);
    QString bddExecutable ("-DROYALE_MOVE_BDD_TO=" + binDir.absolutePath());

    // Start configuration
    QProcess cmakeConfiguration;
    cmakeConfiguration.setProcessChannelMode (QProcess::MergedChannels);
    cmakeConfiguration.setWorkingDirectory (actDir.absolutePath());

#if defined(Q_OS_WIN)

    QString vs_version = QString (getenv ("ROYALE_VS_VERSION"));

    QString generator = "";
    if (vs_version != "")
    {
        generator = "-G \"" + vs_version + "\"";
    }
    cmakeConfiguration.start ("cmake " + generator + " " + royaleSDK + " " + bddExecutable + " ../");
#else
    cmakeConfiguration.start ("cmake " + royaleSDK + " " + bddExecutable + " ../");
#endif

    // Wait till configuration is started
    if (!cmakeConfiguration.waitForStarted (-1))
    {
        return false;
    }

    // Wait till configuration is finished
    if (!cmakeConfiguration.waitForFinished (-1))
    {
        return false;
    }

    QString result = QString (cmakeConfiguration.readAll());
    DLOG (INFO) << result;
    return true;
}

bool CompileHandler::buildBDD()
{
    QDir actDir (m_srcPath);
    if (!actDir.cd ("testing"))
    {
        return false;
    }
    if (!actDir.cd ("bdd"))
    {
        return false;
    }
    if (!actDir.cd ("build"))
    {
        return false;
    }

    // Start build
    QProcess cmakeBuild;
    cmakeBuild.setProcessChannelMode (QProcess::MergedChannels);
    cmakeBuild.setWorkingDirectory (actDir.absolutePath());
    cmakeBuild.start ("cmake --build . --config Release");

    if (!cmakeBuild.waitForStarted (-1))
    {
        return false;
    }

    if (!cmakeBuild.waitForFinished (-1))
    {
        return false;
    }

    QString result = QString (cmakeBuild.readAll());
    DLOG (INFO) << result;

    int16_t errors = checkCompilerOutput (result);
    if (errors < 0)
    {
        return false;
    }

    if (errors > 0)
    {
        // there were errors during compilation
        DLOG (ERROR) << "Compilation failed anywhere, any project caused errors";
        return false;
    }

    // everything worked fine; source was compiled correctly
    DLOG (INFO) << "BDD compilation OK";
    return true;
}

bool CompileHandler::configureSamples()
{
    QDir actDir (m_installPath);
    if (!actDir.cd ("samples"))
    {
        return false;
    }

    QStringList myDirObjects = actDir.entryList ( (QDir::AllDirs | QDir::NoDot | QDir::NoDotDot));

    for (int i = 0; i < myDirObjects.count(); i++)
    {
        // filter: only take the sample directories
        if (myDirObjects.at (i).startsWith ("sample"))
        {
            if (!configureSample (myDirObjects.at (i)))
            {
                return false;
            }
        }
    }
    return true;
}

bool CompileHandler::configureSample (QString sampleName)
{
    QDir actDir (m_installPath);
    if (!actDir.cd ("samples"))
    {
        return false;
    }
    if (!actDir.cd (sampleName))
    {
        return false;
    }

    // Start build
    QProcess cmakeConfiguration;
    cmakeConfiguration.setProcessChannelMode (QProcess::MergedChannels);
    cmakeConfiguration.setWorkingDirectory (actDir.absolutePath());

#if defined(Q_OS_WIN)

    QString vs_version = QString (getenv ("ROYALE_VS_VERSION"));

    QString generator = "";
    if (vs_version != "")
    {
        generator = "-G \"" + vs_version + "\"";
    }
    cmakeConfiguration.start ("cmake " + generator + " .");
#else
    cmakeConfiguration.start ("cmake .");
#endif

    // Wait till configuration is started
    if (!cmakeConfiguration.waitForStarted (-1))
    {
        return false;
    }

    // Wait till configuration is finished
    if (!cmakeConfiguration.waitForFinished (-1))
    {
        return false;
    }

    QString result = QString (cmakeConfiguration.readAll());
    DLOG (INFO) << result;
    return true;
}

bool CompileHandler::buildSamples()
{
    QDir actDir (m_installPath);
    if (!actDir.cd ("samples"))
    {
        return false;
    }

    QStringList myDirObjects = actDir.entryList ( (QDir::AllDirs | QDir::NoDot | QDir::NoDotDot));

    for (int i = 0; i < myDirObjects.count(); i++)
    {
        // filter: only take the sample directories
        if (myDirObjects.at (i).startsWith ("sample"))
        {
            if (!buildSample (myDirObjects.at (i)))
            {
                return false;
            }
        }
    }
    return true;
}

bool CompileHandler::buildSample (QString sampleName)
{
    QDir actDir (m_installPath);
    if (!actDir.cd ("samples"))
    {
        return false;
    }
    if (!actDir.cd (sampleName))
    {
        return false;
    }

    // Start build
    QProcess cmakeBuild;
    cmakeBuild.setProcessChannelMode (QProcess::MergedChannels);
    cmakeBuild.setWorkingDirectory (actDir.absolutePath());
    cmakeBuild.start ("cmake --build . --config Release");

    if (!cmakeBuild.waitForStarted (-1))
    {
        return false;
    }

    if (!cmakeBuild.waitForFinished (-1))
    {
        return false;
    }

    QString result = QString (cmakeBuild.readAll());
    DLOG (INFO) << result;

    int16_t errors = checkCompilerOutput (result);
    if (errors < 0)
    {
        return false;
    }

    if (errors > 0)
    {
        // there were errors during compilation
        DLOG (ERROR) << "Compilation failed anywhere, any project caused errors";
        return false;
    }

    // everything worked fine; source was compiled correctly
    DLOG (INFO) << "Sample compilation OK";
    return true;
}
