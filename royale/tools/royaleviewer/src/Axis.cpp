/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "Axis.hpp"

#include <QVector3D>
#include <QVector4D>
#include <QOpenGLShaderProgram>

struct VertexData
{
    QVector3D position;
    QVector4D color;
};

Axis::Axis()
    : Renderable()
{
    m_numVtxs = 6;
    m_numIdxs = 6;
    m_vertices = new VertexData [m_numVtxs];
    m_indices = new GLushort[m_numIdxs];
    m_program = nullptr;

}

Axis::~Axis()
{
    // subclass-specific destructor
    if (m_vertices)
    {
        delete [] m_vertices;
    }

    if (m_indices)
    {
        delete[] m_indices;
    }

    delete m_program;
    m_program = nullptr;
}

bool Axis::initGL()
{


    if (!Renderable::initGL())
    {
        return false;
    }

    initGeometry();

    // setup shaders
    QOpenGLShader vshader (QOpenGLShader::Vertex);
    const char *vsrc =
#ifdef TARGET_PLATFORM_ANDROID
        "precision mediump float;                               \n"
#endif
        "attribute vec4 vPosition;                              \n"
        "attribute vec4 colorVec;                               \n"
        "uniform mat4 uMVP;			                            \n"
        "uniform mat4 uModelTransform;			                \n"
        "varying vec4 vCol;			                            \n"
        "                                                       \n"
        "void main() {                                          \n"
        "  vCol = colorVec;                                     \n"
        "  gl_Position = uMVP * uModelTransform  * vPosition;   \n"
        "}                                                      \n";
    if (!vshader.compileSourceCode (vsrc))
    {
        return false;
    }

    QOpenGLShader fshader (QOpenGLShader::Fragment);
    const char *fsrc =
#ifdef TARGET_PLATFORM_ANDROID
        "precision mediump float;                               \n"
#endif
        "varying vec4 vCol;                                     \n"
        "                                                       \n"
        "void main() {                                          \n"
        "  gl_FragColor = vCol;                                 \n"
        "}                                                      \n";
    if (!fshader.compileSourceCode (fsrc))
    {
        return false;
    }

    m_program = new QOpenGLShaderProgram;
    m_program->addShader (&vshader);
    m_program->addShader (&fshader);

    // Link shader pipeline
    if (!m_program->link())
    {
        return false;
    }



    return true;
}

void Axis::render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix,  const QMatrix4x4 &modelTransform)
{
    //GLint glVersion[2];
    //glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    //glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    //printf ("Axis::draw()...\n");

    //glEnable (GL_DEPTH_TEST);
    glLineWidth (4.0f);

    //QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    // Bind shader pipeline for use
    m_program->bind();
    m_arrayBuf.bind();

    m_arrayBuf.write (0, m_vertices, static_cast<int> (m_numVtxs * sizeof (VertexData)));
    m_indexBuf.bind();
    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = m_program->attributeLocation ("vPosition");
    m_program->enableAttributeArray (vertexLocation);
    m_program->setAttributeBuffer (vertexLocation, GL_FLOAT, static_cast<int> (offset), 3, sizeof (VertexData));

    // Offset for color information
    offset += sizeof (QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = m_program->attributeLocation ("colorVec");
    m_program->enableAttributeArray (colorLocation);
    m_program->setAttributeBuffer (colorLocation, GL_FLOAT, static_cast<int> (offset), 4, sizeof (VertexData));

    const QMatrix4x4 mvpMatrix = projectionMatrix * mvMatrix;
    m_program->setUniformValue ("uMVP", mvpMatrix);
    m_program->setUniformValue ("uModelTransform", modelTransform);

    // Draw cube geometry using indices from VBO 1
    glDrawElements (GL_LINES, m_numIdxs, GL_UNSIGNED_SHORT, 0);

    // reset
    m_program->disableAttributeArray (vertexLocation);
    m_program->disableAttributeArray (colorLocation);
    m_indexBuf.release();
    m_arrayBuf.release();
    m_program->release();

    glLineWidth (1.0f);
}


void Axis::initGeometry()
{
    Renderable::initGeometry();
    m_vertices[0] = { QVector3D (0.f, 0.f, 0.f), QVector4D (1.0f, 0.f, 0.f, 1.0f) };
    m_vertices[1] = { QVector3D (0.2f, 0.f, 0.f), QVector4D (1.0f, 0.f, 0.f, 1.0f) };
    m_vertices[2] = { QVector3D (0.f, 0.f, 0.f), QVector4D (0.0f, 1.f, 0.f, 1.0f) };
    m_vertices[3] = { QVector3D (0.f, -0.2f, 0.f), QVector4D (0.0f, 1.f, 0.f, 1.0f) };
    m_vertices[4] = { QVector3D (0.f, 0.f, 0.f), QVector4D (0.0f, 0.f, 1.f, 1.0f) };
    m_vertices[5] = { QVector3D (0.f, 0.f, -0.2f), QVector4D (0.0f, 0.f, 1.f, 1.0f) };

    for (GLushort i = 0; i < 6; ++i)
    {
        m_indices[i] = i;
    }

    m_arrayBuf.bind();
    m_arrayBuf.allocate (m_vertices, static_cast<int> (m_numVtxs * sizeof (VertexData)));

    m_indexBuf.bind();
    m_indexBuf.allocate (m_indices, static_cast<int> (m_numIdxs * sizeof (GLushort)));
}
