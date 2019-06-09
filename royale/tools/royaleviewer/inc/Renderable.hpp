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

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

QT_FORWARD_DECLARE_CLASS (QOpenGLShaderProgram);

class Renderable : protected QOpenGLFunctions
{
public:
    Renderable();
    virtual ~Renderable();

    virtual bool initGL ();
    virtual void render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform) = 0;

protected:
    virtual void initGeometry() {};

    QOpenGLShaderProgram *m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_arrayBuf;
    QOpenGLBuffer m_indexBuf;
    GLsizei m_numIdxs;
    GLsizei m_numVtxs;
};
