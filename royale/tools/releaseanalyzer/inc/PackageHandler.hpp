/***************************************************************************
**                                                                        **
**  Copyright (C) 2015 Infineon Technologies                              **
**  All rights reserved.                                                  **
**                                                                        **
****************************************************************************/

#ifndef _PACKAGE_HANDLER_
#define _PACKAGE_HANDLER_

#include <QString>
#include <QDebug>
#include <QTextStream>
#include <QVector>
#include <cstdint>

namespace releaseanalyzer
{
    enum class CheckControlFlow : uint16_t
    {
        Unzip = 0,
        ParseDir = 1,
        VerifyWithFile = 2,
        ConfigureSamples = 3,
        CompileSamples = 4,
        CopyLibs = 5,
        ConfigureBDD = 6,
        CompileBDD = 7,
        RunBDD = 8,
        RunCucumber = 9
    };

    class PackageHandler : public QObject
    {
        Q_OBJECT

    public:
        PackageHandler (QObject *parent = 0);
        virtual ~PackageHandler();

        uint32_t runValidation (int argc, char *argv[]);

        uint32_t getControlFlow();
        void printControlFlow();
        void setDryRun (bool);
        static QString getPlatformType();
        static QString getCompileTimePlatform();

    private:

        uint32_t m_mode, m_workflow, m_error, m_output;
        static const char *controlFlowMessages[];
        QString m_errorMessage;

        QString         m_referenceStructurePath;
        QString         m_referenceFileName;
        QString         m_unpackPath;
        QString         m_sevenZipPath;
        QString         m_checkPath;
        QString         m_packagePath;
        QString         m_validationFileName;
        QString         m_differenceFile;
        QString         m_settingsFileName;
        QString         m_useCreateConfig;
        QStringList     m_ignoredPaths;

        QTextStream *m_parseStream;
        bool m_withoutRootDirName;
        bool m_dryRun;
        bool m_generateReferenceFiles;
        bool m_debugON;
        bool m_fullAutomatic;

        QVector<QString> m_parsedVector;
        QVector<QString> m_expectedVector;

        void controlFlow (const uint32_t state);
        uint8_t inline dumpMissingCases (uint32_t state);
        uint8_t inline getMSB (uint32_t number);
        bool compareFiles (QString file1Name, QString file1Path, QString file2Name, QString file2Path);
        bool compareVectors();
        uint16_t parseSettingsFile();

        void unzipPackage();
        void parseDir (QString curDirPath);
        void parseDirRecursivly (QString curDirPath, const QString &operatingMainPath);

        bool checkArgumentCombination();
        uint32_t validatePackage();

        uint32_t compileAgainstDeliveredLib();
        uint32_t runBDD();

    public slots:
        void readOutput();

    };
}

#endif //_PACKAGE_HANDLER_
