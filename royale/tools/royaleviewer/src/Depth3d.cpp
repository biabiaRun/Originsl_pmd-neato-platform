/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "Depth3d.hpp"

#include <QVector3D>
#include <QVector4D>
#include <QOpenGLShaderProgram>

#include <cmath>

struct VertexData
{
    //QVector3D position;
    QVector3D position;
    QVector4D color;
};

using namespace royale;

Depth3d::Depth3d (ColorHelper *colorHelper)
    : Renderable(),
      m_vertices (NULL),
      m_indices (NULL),
      m_isGLInit (false),
      m_isDataAvailable (false),
      m_isNewDataAvailable (false),
      m_forceColorRangeUpdate (false),
      m_showDistance (true),
      m_uniform (false),
      m_showGrayimage (false),
      m_showOverlayimage (false),
      m_modeSwitched (false),
      m_pointScaleFactor (1.f),
      m_lensParamsChanged (false),
      m_focalLengthInPxSizeY (212.f),
      m_screenResYHalf (240.f),
      m_colorHelper (colorHelper),
      m_filterMin (0.0f),
      m_filterMax (7.5f)
{
}

Depth3d::~Depth3d()
{
    if (m_indices)
    {
        delete[] m_indices;
    }

    if (m_vertices)
    {
        delete [] m_vertices;
    }
}

bool Depth3d::initGL()
{
    m_arrayBuf.setUsagePattern (QOpenGLBuffer::DynamicDraw);
    if (!Renderable::initGL())
    {
        return false;
    }

    initGeometry();

    if (m_program == nullptr)
    {
        // setup shaders
        m_program = new QOpenGLShaderProgram;
        QOpenGLShader vshader (QOpenGLShader::Vertex);

        const char *vsrc =
#ifdef TARGET_PLATFORM_ANDROID
            "precision mediump float;                                                       \n"
#endif
            " attribute vec4 vPosition;                                                     \n"
            " attribute vec4 colorVec;                                                      \n"
            " //uniform mat4 uMVP;			                                                \n"
            " uniform mat4 uMV;			                                                    \n"
            " uniform mat4 uProjection;			                                            \n"
            " uniform mat4 uModelTransform;			                                        \n"
            " uniform float uPointScaleFactor;		                                        \n"
            " uniform float uFocalLengthInPxSizeY;	                                        \n"
            " varying vec4 vCol;			                                                \n"
            "                                                                               \n"
            " void main() {                                                                 \n"
            " if (vPosition.x < 0.001 && vPosition.y < 0.001  && vPosition.z < 0.001)       \n"
            " {                                                                             \n"
            "      vCol = vec4(0,0,0,0);                                                    \n"
            " }                                                                             \n"
            " else                                                                          \n"
            " {                                                                             \n"
            "       vCol = colorVec;                                                        \n"
            " }                                                                             \n"
            " vec4 newPos = vec4( vPosition.x, -1.0*vPosition.y, -1.0*vPosition.z, 1.0 );   \n"
            "  	                                                                            \n"
            " // world pos 	                                                                \n"
            " vec4 wpos = uModelTransform * newPos;	                                        \n"
            " gl_Position = uProjection * uMV * wpos;                                       \n"
            " vec4 eyeSpacePos = uMV * wpos;	                                            \n"
            " float dist = length(eyeSpacePos.xyz); 	                                    \n"
            " 	                                                                            \n"
            " // calculate diameter in world space	                                        \n"
            " float curDiameter = vPosition.z/uFocalLengthInPxSizeY;	                    \n"
            " 	                                                                            \n"
            " // use factor for visual improvement	                                        \n"
            " float scaleFactor = 0.8;                                                      \n"
            " gl_PointSize =  scaleFactor * curDiameter * (uPointScaleFactor/dist);         \n"
            " if (gl_PointSize < 1.0) 	                                                    \n"
            "    gl_PointSize = 1.0; 	                                                    \n"
            "}                                                                              \n";

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
            "  if (vCol.w < 0.00001)                                \n"
            "  {                                                    \n"
            "     discard;                                          \n"
            "  }                                                    \n"
            "                                                       \n"
            "  gl_FragColor = vCol;                                 \n"
            "}                                                      \n";
        if (!fshader.compileSourceCode (fsrc))
        {
            return false;
        }

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
    }

    m_isGLInit = true;
    return true;
}

void Depth3d::render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform)
{
    // init only if proper data has arrived in order to fetch correct data sizes
    if (!m_isGLInit && m_isDataAvailable)
    {
        initGL();
    }
    // do nothing if not ready yet for rendering
    if (!m_isGLInit)
    {
        return;
    }

    //printf ("Depth3d::draw()...\n");

    glLineWidth (1.0f);

#ifndef TARGET_PLATFORM_ANDROID
    glEnable (GL_POINT_SPRITE);
    glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);
    //glEnable (GL_POINT_SIZE);
#endif

    //QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    // sync data and update for proper rendering
    if (m_isNewDataAvailable || (m_isDataAvailable && m_modeSwitched))
    {
        if (m_numIdxs != m_currentDataset->height * m_currentDataset->width)
        {
            if (m_vertices)
            {
                delete[] m_vertices;
            }

            if (m_indices)
            {
                delete[] m_indices;
            }

            initGL();

            m_numVtxs = m_currentDataset->height * m_currentDataset->width;
            m_numIdxs = m_currentDataset->height * m_currentDataset->width;

            m_vertices = new VertexData[m_numVtxs];
            m_indices = new GLuint[m_numIdxs];

            for (GLsizei i = 0; i < m_numIdxs; ++i)
            {
                m_indices[i] = i;
            }

            m_indexBuf.bind();
            m_indexBuf.write (0, m_indices, static_cast<int> (m_numIdxs * sizeof (GLuint)));
        }

        m_arrayBuf.bind();


        if (m_showDistance)
        {
            for (GLsizei i = 0; i < m_numVtxs; ++i)
            {
                // update new coordinates
                const DepthPoint &currentPoint = m_currentDataset->points[i];
                const IntermediatePoint &currentInterPoint = m_currentInterDataset->points[i];
                m_vertices[i].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                const RgbColor &curColor = m_colorHelper->getColor (currentPoint.z);
                if (currentInterPoint.flags & FLAGS_SBI_ON)
                {
                    m_vertices[i].color = QVector4D (0.5f, 0.5f, 0.5f, 1.0f);
                }
                else if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    m_vertices[i].color = QVector4D (0.0f, 0.0f, 0.0f, 1.0f);
                }
                else
                {
                    m_vertices[i].color = QVector4D (curColor.r / 255.0f, curColor.g / 255.0f, curColor.b / 255.0f, 1.0f);
                }
            }
        }
        else if (m_showGrayimage)
        {
            for (GLsizei i = 0; i < m_numVtxs; ++i)
            {
                // update new coordinates
                const DepthPoint &currentPoint = m_currentDataset->points[i];
                m_vertices[i].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                const RgbColor &curColor = m_colorHelper->getGrayColor (currentPoint.grayValue);
                if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    m_vertices[i].color = QVector4D (0.0f, 0.0f, 0.0f, 0.5f);
                }
                else
                {
                    if (m_uniform)
                    {
                        m_vertices[i].color = QVector4D (210.0f / 255.0f, 210.0f / 255.0f, 210.0f / 255.0f, 0.5f);
                    }
                    else
                    {
                        m_vertices[i].color = QVector4D (curColor.r / 255.0f, curColor.g / 255.0f, curColor.b / 255.0f, 0.5f);
                    }
                }
            }
        }
        else if (m_showOverlayimage)
        {
            for (GLsizei i = 0; i < m_numVtxs; ++i)
            {
                // update new coordinates
                const DepthPoint &currentPoint = m_currentDataset->points[i];
                m_vertices[i].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                const RgbColor &curColor = m_colorHelper->getColor (currentPoint.z);

                float grayVal = (float) m_colorHelper->getGrayColor (currentPoint.grayValue).r;
                float scaleTmp = 8.0f * std::pow (currentPoint.z, 1.5f) * grayVal / 255.f;
                if (scaleTmp != 0.f)
                {
                    scaleTmp += 0.1f;
                }
                float scale = qBound (0.f, scaleTmp, 1.f);

                if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    m_vertices[i].color = QVector4D (0.0f, 0.0f, 0.0f, 1.0f);
                }
                else
                {
                    m_vertices[i].color = QVector4D (scale * curColor.r / 255.0f, scale * curColor.g / 255.0f, scale * curColor.b / 255.0f, 1.0f);
                }
            }
        }

        m_arrayBuf.write (0, m_vertices, static_cast<int> (m_numVtxs * sizeof (VertexData)));
        // reset
        m_isNewDataAvailable = false;
        m_modeSwitched = false;
    }


    if (m_lensParamsChanged)
    {
        m_focalLengthInPxSizeY =  m_lensParam.focalLength.first;
        m_lensParamsChanged = false;
    }

    // Tell OpenGL which VBOs to use
    m_arrayBuf.bind();
    m_indexBuf.bind();

    // Offset for position
    quintptr offset = 0;
    int vertexLocation = m_program->attributeLocation ("vPosition");
    m_program->enableAttributeArray (vertexLocation);
    m_program->setAttributeBuffer (vertexLocation, GL_FLOAT, static_cast<int> (offset), 3, sizeof (VertexData));


    // Offset for color information
    offset += sizeof (QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = m_program->attributeLocation ("colorVec");
    m_program->enableAttributeArray (colorLocation);
    m_program->setAttributeBuffer (colorLocation, GL_FLOAT, static_cast<int> (offset), 4, sizeof (VertexData));

    m_program->setUniformValue ("uMV", mvMatrix);
    m_program->setUniformValue ("uProjection", projectionMatrix);
    m_program->setUniformValue ("uModelTransform", modelTransform);
    m_program->setUniformValue ("uPointScaleFactor", m_pointScaleFactor);
    m_program->setUniformValue ("uFocalLengthInPxSizeY", m_focalLengthInPxSizeY);

    // Draw cube geometry using indices from VBO 1
    glDrawElements (GL_POINTS, m_numIdxs, GL_UNSIGNED_INT, 0);

#ifndef TARGET_PLATFORM_ANDROID
    glDisable (GL_POINT_SPRITE);
    glDisable (GL_VERTEX_PROGRAM_POINT_SIZE);
//glDisable (GL_POINT_SIZE);
#endif

// reset
    m_program->disableAttributeArray (vertexLocation);
    m_program->disableAttributeArray (colorLocation);
    m_indexBuf.release();
    m_arrayBuf.release();

    m_program->release();
}


void Depth3d::initGeometry ()
{
    Renderable::initGeometry();
    m_vertices = new VertexData[m_currentDataset->height * m_currentDataset->width];
    m_indices = new GLuint[m_currentDataset->height * m_currentDataset->width];

    m_numVtxs = m_currentDataset->height * m_currentDataset->width;
    m_numIdxs = m_currentDataset->height * m_currentDataset->width;

    for (GLsizei i = 0; i < m_numVtxs; ++i)
    {
        float defVal = 0.0f;
        m_vertices[i].position = QVector3D (defVal, defVal, defVal);
    }

    for (GLsizei i = 0; i < m_numIdxs; ++i)
    {
        m_indices[i] = i;
    }

    // Transfer vertex data to VBO 0
    m_arrayBuf.bind();
    m_arrayBuf.allocate (m_vertices, static_cast<int> (m_numVtxs * sizeof (VertexData)));

    // Transfer index data to VBO 1
    m_indexBuf.bind();
    m_indexBuf.allocate (m_indices, static_cast<int> (m_numIdxs * sizeof (GLuint)));

}


void Depth3d::setData (const royale::DepthData *data, const royale::IntermediateData *intData)
{
    // no sync needed - sync resides in manager class "3DView"
    m_currentDataset = data;
    m_currentInterDataset = intData;
    // only important for the first time
    m_isNewDataAvailable = true;
    m_isDataAvailable = true;
}

void Depth3d::updateLensParameters (royale::LensParameters &lensParam)
{
    m_lensParam = lensParam;
    m_lensParamsChanged = true;
}

void Depth3d::switchToDistanceBuffer()
{
    m_showDistance = true;
    m_uniform = false;
    m_showGrayimage = false;
    m_showOverlayimage = false;
    m_modeSwitched = true;
}

void Depth3d::switchToAmplitudeBuffer()
{
    // obsolete - to remove!
    m_showDistance = false;
    m_modeSwitched = true;
}

void Depth3d::switchToGrayBuffer (bool uniform)
{
    m_showDistance = false;
    m_uniform = uniform;
    m_showGrayimage = true;
    m_showOverlayimage = false;
    m_modeSwitched = true;
}

void Depth3d::switchToOverlay()
{
    m_showDistance = false;
    m_uniform = false;
    m_showGrayimage = false;
    m_showOverlayimage = true;
    m_modeSwitched = true;
}

void Depth3d::colorRangeChanged()
{
    m_modeSwitched = true;
}

void Depth3d::setFilterMinMax (float filterMin, float filterMax, bool cameraStart)
{
    m_filterMin = filterMin;
    m_filterMax = filterMax;
    if (!cameraStart)
    {
        m_isNewDataAvailable = true;
    }
}

void Depth3d::setPointScaleFactor (const float &val)
{

    m_pointScaleFactor = val;
}

void Depth3d::setScreenResYHalf (const float &val)
{
    m_screenResYHalf = val;
}
