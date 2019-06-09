/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "Renderable.hpp"

#include <QOpenGLShaderProgram>


Renderable::Renderable()
    : m_program (NULL),
      m_indexBuf (QOpenGLBuffer::IndexBuffer),
      m_numIdxs (0),
      m_numVtxs (0)
{

}

Renderable::~Renderable()
{
    if (m_program)
    {
        delete m_program;
        m_program = nullptr;
    }

    m_arrayBuf.destroy();
    m_indexBuf.destroy();
}

bool Renderable::initGL()
{
    initializeOpenGLFunctions();

    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder (&m_vao);

    // Generate 2 VBOs
    m_arrayBuf.create();
    m_indexBuf.create();

    return true;
}

