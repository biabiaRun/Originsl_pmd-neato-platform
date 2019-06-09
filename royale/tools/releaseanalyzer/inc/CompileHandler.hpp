/***************************************************************************
**                                                                        **
**  Copyright (C) 2015 Infineon Technologies                              **
**  All rights reserved.                                                  **
**                                                                        **
****************************************************************************/

#ifndef _COMPILE_HANDLER_
#define _COMPILE_HANDLER_

#include <QString>
#include <QDebug>
#include <cstdint>

namespace releaseanalyzer
{
    class CompileHandler : public QObject
    {
        Q_OBJECT

    public:
        CompileHandler (QString installPath, QString srcPath, QObject *parent = 0);
        virtual ~CompileHandler();

        void setDebug (bool debug);

        bool buildBDD();
        bool configureBDD();

        bool configureSamples();
        bool configureSample (QString sampleName);

        bool buildSamples();
        bool buildSample (QString sampleName);

    private:
        bool m_debug;
        QString m_installPath;
        QString m_srcPath;
    };
}

#endif //_COMPILE_HANDLER_
