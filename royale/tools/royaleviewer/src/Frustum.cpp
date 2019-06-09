/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "Frustum.hpp"

#include <QVector3D>
#include <QVector4D>
#include <QOpenGLShaderProgram>

struct VertexData
{
    QVector3D position;
    QVector4D color;
};

Frustum::Frustum()
    : Renderable(),
      m_vertices (NULL),
      m_indices (NULL)
{
    m_numVtxs = 17;
    m_numIdxs = 40;

    m_vertices = new VertexData[m_numVtxs];
    m_indices = new GLushort[m_numIdxs];
    m_program = nullptr;
}

Frustum::~Frustum()
{
    // subclass-specific destructor
    if (m_vertices)
    {
        delete[] m_vertices;
    }

    if (m_indices)
    {
        delete[] m_indices;
    }

    delete m_program;
    m_program = nullptr;
}

bool Frustum::initGLWithFov (const float fovx, const float fovy)
{
    if (!Renderable::initGL())
    {
        return false;
    }

    initGeometry (fovx, fovy);

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
        "  gl_Position = uMVP * uModelTransform * vPosition; 	\n"
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
        "varying vec4 vCol;                                   \n"
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

    /*
    // Bind shader pipeline for use
    if (!m_program->bind())
    {
        return false;
    }
    */

    return true;
}

void Frustum::render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform)
{
    //printf ("Frustum::draw()...\n");

    //glEnable (GL_DEPTH_TEST);
    glLineWidth (1.0f);

    m_program->bind();

    // Tell OpenGL which VBOs to use
    m_arrayBuf.bind();
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
}


void Frustum::initGeometry (const float fovx, const float fovy)
{
    Renderable::initGeometry();

    float tmpFovx = fovx / 100.0f;
    float tmpFovy = fovy / 100.0f;

    QVector4D color1 = QVector4D (0.7f, 0.7f, 0.7f, 1.0f);
    QVector4D color2 = QVector4D (0.3f, 0.3f, 0.3f, 1.0f); // 0.2f, 0.2f, 0.2f, 1.0f

    m_vertices[0] = { QVector3D (0.f, 0.f, 0.f), QVector4D (color1) }; // origin   [0]

    m_vertices[1] = { QVector3D (-4.0f * tmpFovx, 4.0f * tmpFovy, -4.0f), QVector4D (color1) }; // top left: far: (x_n*z_f)/z_n = x_f   // [1]
    m_vertices[2] = { QVector3D (-4.0f * tmpFovx, -4.0f * tmpFovy, -4.0f), QVector4D (color1) }; // bottom left: far                     // [2]
    m_vertices[3] = { QVector3D (4.0f * tmpFovx, -4.0f * tmpFovy, -4.0f), QVector4D (color1) }; // bottom right: far                    // [3]
    m_vertices[4] = { QVector3D (4.0f * tmpFovx, 4.0f * tmpFovy, -4.0f), QVector4D (color1) }; // top right: far                       // [4]

    // at 1.0 m dist
    m_vertices[5] = { QVector3D (-tmpFovx, tmpFovy, -1.0f), QVector4D (color2) }; // [5]
    m_vertices[6] = { QVector3D (-tmpFovx, -tmpFovy, -1.0f), QVector4D (color2) }; // [6]
    m_vertices[7] = { QVector3D (tmpFovx, -tmpFovy, -1.0f), QVector4D (color2) }; // [7]
    m_vertices[8] = { QVector3D (tmpFovx, tmpFovy, -1.0f), QVector4D (color2) }; // [8]

    // at 2.0 m dist
    m_vertices[9] = { QVector3D (-2.0f * tmpFovx, 2.0f * tmpFovy, -2.0f), QVector4D (color2) }; // [9]
    m_vertices[10] = { QVector3D (-2.0f * tmpFovx, -2.0f * tmpFovy, -2.0f), QVector4D (color2) }; // [10]
    m_vertices[11] = { QVector3D (2.0f * tmpFovx, -2.0f * tmpFovy, -2.0f), QVector4D (color2) }; // [11]
    m_vertices[12] = { QVector3D (2.0f * tmpFovx, 2.0f * tmpFovy, -2.0f), QVector4D (color2) }; // [12]

    // at 3.0 m dist
    m_vertices[13] = { QVector3D (-3.0f * tmpFovx, 3.0f * tmpFovy, -3.0f), QVector4D (color2) }; // [13]
    m_vertices[14] = { QVector3D (-3.0f * tmpFovx, -3.0f * tmpFovy, -3.0f), QVector4D (color2) }; // [14]
    m_vertices[15] = { QVector3D (3.0f * tmpFovx, -3.0f * tmpFovy, -3.0f), QVector4D (color2) }; // [15]
    m_vertices[16] = { QVector3D (3.0f * tmpFovx, 3.0f * tmpFovy, -3.0f), QVector4D (color2) }; // [16]


    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 0;
    m_indices[3] = 2;
    m_indices[4] = 0;
    m_indices[5] = 3;
    m_indices[6] = 0;
    m_indices[7] = 4;
    m_indices[8] = 1;
    m_indices[9] = 2;
    m_indices[10] = 2;
    m_indices[11] = 3;
    m_indices[12] = 3;
    m_indices[13] = 4;
    m_indices[14] = 4;
    m_indices[15] = 1;
    m_indices[16] = 5;
    m_indices[17] = 6;
    m_indices[18] = 6;
    m_indices[19] = 7;
    m_indices[20] = 7;
    m_indices[21] = 8;
    m_indices[22] = 8;
    m_indices[23] = 5;
    m_indices[24] = 9;
    m_indices[25] = 10;
    m_indices[26] = 10;
    m_indices[27] = 11;
    m_indices[28] = 11;
    m_indices[29] = 12;
    m_indices[30] = 12;
    m_indices[31] = 9;
    m_indices[32] = 13;
    m_indices[33] = 14;
    m_indices[34] = 14;
    m_indices[35] = 15;
    m_indices[36] = 15;
    m_indices[37] = 16;
    m_indices[38] = 16;
    m_indices[39] = 13;

    // Transfer vertex data to VBO 0
    m_arrayBuf.bind();
    m_arrayBuf.allocate (m_vertices, static_cast<int> (m_numVtxs * sizeof (VertexData)));

    // Transfer index data to VBO 1
    m_indexBuf.bind();
    m_indexBuf.allocate (m_indices, static_cast<int> (m_numIdxs * sizeof (GLushort)));
}

void Frustum::updateFov (const float fovx, const float fovy)
{
    initGeometry (fovx, fovy);
}
