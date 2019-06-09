/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <QtWidgets>
#include <QtOpenGL>
#include <royale/DepthData.hpp>
#include <royale/IntermediateData.hpp>
#include "ColorHelper.hpp"

#ifdef ANDROID
#include <GLES/gl.h>
#endif

class IView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    IView (ColorHelper *colorHelper, QMutex *dataMutex, QWidget *parent = 0, Qt::WindowFlags f = 0)
        : QOpenGLWidget (parent, f),
          m_colorHelper (colorHelper),
          m_dataMutex (dataMutex)
    {}

    virtual ~IView() {}

    virtual void colorRangeChanged() = 0;

public slots:
    virtual void onNewData (const royale::DepthData *data, const royale::IntermediateData *intData)
    {
        // NOTE: implement in subclass.
    }

signals:

    void newData (const royale::DepthData *data, const royale::IntermediateData *intData);

protected:
    virtual void initializeGL()
    {
        initializeOpenGLFunctions();
    }

    ColorHelper *m_colorHelper;
    QMutex *m_dataMutex;
};
