/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "Depth3dSamsungS7.hpp"

#include <QVector3D>
#include <QVector4D>
#include <QOpenGLShaderProgram>

#include <cmath>

struct VertexData
{
    QVector3D position;
    QVector4D color;
};

using namespace royale;

Depth3dSamsungS7::Depth3dSamsungS7 (ColorHelper *colorHelper)
    : Depth3d (colorHelper)
{

}

Depth3dSamsungS7::~Depth3dSamsungS7()
{
}


void Depth3dSamsungS7::render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform)
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

    //QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    // sync data and update for proper rendering
    if (m_isNewDataAvailable || (m_isDataAvailable && m_modeSwitched))
    {
        if (m_numIdxs != m_currentDataset->height * m_currentDataset->width * 6)
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

            m_numVtxs = m_currentDataset->height * m_currentDataset->width * 6;
            m_numIdxs = m_currentDataset->height * m_currentDataset->width * 6;

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

        const float picoFlexxPixelScale = 0.006f;

        if (m_showDistance)
        {
            for (GLsizei i = 0; i < m_numVtxs / 6; ++i)
            {
                // update new coordinates
                const DepthPoint &currentPoint = m_currentDataset->points[i];
                const IntermediatePoint &currentInterPoint = m_currentInterDataset->points[i];
                const float pixelScale = picoFlexxPixelScale * currentPoint.z;
                m_vertices[6 * i + 0].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                m_vertices[6 * i + 1].position = QVector3D (currentPoint.x, currentPoint.y + pixelScale, currentPoint.z);
                m_vertices[6 * i + 2].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y + pixelScale, currentPoint.z);

                m_vertices[6 * i + 3].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                m_vertices[6 * i + 4].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y + pixelScale, currentPoint.z);
                m_vertices[6 * i + 5].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y, currentPoint.z);
                RgbColor curColor = m_colorHelper->getColor (currentPoint.z);
                if (currentInterPoint.flags & FLAGS_SBI_ON)
                {
                    curColor.r = curColor.g = curColor.b = 120;
                }
                else if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    curColor.r = curColor.g = curColor.b = 0;
                }
                const QVector4D color (curColor.r / 255.0f, curColor.g / 255.0f, curColor.b / 255.0f, 1.0f);
                m_vertices[6 * i].color = color;
                m_vertices[6 * i + 1].color = color;
                m_vertices[6 * i + 2].color = color;
                m_vertices[6 * i + 3].color = color;
                m_vertices[6 * i + 4].color = color;
                m_vertices[6 * i + 5].color = color;
            }

        }
        else if (m_showGrayimage)
        {
            for (GLsizei i = 0; i < m_numVtxs / 6; ++i)
            {
                // update new coordinates
                const DepthPoint &currentPoint = m_currentDataset->points[i];
                const float pixelScale = picoFlexxPixelScale * currentPoint.z;
                m_vertices[6 * i + 0].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                m_vertices[6 * i + 1].position = QVector3D (currentPoint.x, currentPoint.y + pixelScale, currentPoint.z);
                m_vertices[6 * i + 2].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y + pixelScale, currentPoint.z);

                m_vertices[6 * i + 3].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                m_vertices[6 * i + 4].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y + pixelScale, currentPoint.z);
                m_vertices[6 * i + 5].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y, currentPoint.z);
                RgbColor curColor = m_colorHelper->getGrayColor (currentPoint.grayValue);
                if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    curColor.r = curColor.g = curColor.b = 0;
                }
                else if (m_uniform)
                {
                    curColor.r = curColor.g = curColor.b = 210;
                }
                const QVector4D color (curColor.r / 255.0f, curColor.g / 255.0f, curColor.b / 255.0f, 0.5f);
                m_vertices[6 * i].color = color;
                m_vertices[6 * i + 1].color = color;
                m_vertices[6 * i + 2].color = color;
                m_vertices[6 * i + 3].color = color;
                m_vertices[6 * i + 4].color = color;
                m_vertices[6 * i + 5].color = color;
            }
        }
        else if (m_showOverlayimage)
        {
            for (GLsizei i = 0; i < m_numVtxs / 6; ++i)
            {
                // update new coordinates
                const DepthPoint &currentPoint = m_currentDataset->points[i];
                const float pixelScale = picoFlexxPixelScale * currentPoint.z;
                m_vertices[6 * i + 0].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                m_vertices[6 * i + 1].position = QVector3D (currentPoint.x, currentPoint.y + pixelScale, currentPoint.z);
                m_vertices[6 * i + 2].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y + pixelScale, currentPoint.z);

                m_vertices[6 * i + 3].position = QVector3D (currentPoint.x, currentPoint.y, currentPoint.z);
                m_vertices[6 * i + 4].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y + pixelScale, currentPoint.z);
                m_vertices[6 * i + 5].position = QVector3D (currentPoint.x + pixelScale, currentPoint.y, currentPoint.z);
                RgbColor curColor = m_colorHelper->getColor (currentPoint.z);
                if (currentPoint.z == 0.f || currentPoint.z < m_filterMin || currentPoint.z > m_filterMax)
                {
                    curColor.r = curColor.g = curColor.b = 0;
                }

                float grayVal = (float) m_colorHelper->getGrayColor (currentPoint.grayValue).r;
                float scaleTmp = 8.0f * std::pow (currentPoint.z, 1.5f) * grayVal / 255.f;
                if (scaleTmp != 0.f)
                {
                    scaleTmp += 0.1f;
                }
                float scale = qBound (0.f, scaleTmp, 1.f);

                curColor.r = static_cast<unsigned char> (scale * (float) curColor.r);
                curColor.g = static_cast<unsigned char> (scale * (float) curColor.g);
                curColor.b = static_cast<unsigned char> (scale * (float) curColor.b);

                const QVector4D color (curColor.r / 255.0f, curColor.g / 255.0f, curColor.b / 255.0f, 1.0f);
                m_vertices[6 * i].color = color;
                m_vertices[6 * i + 1].color = color;
                m_vertices[6 * i + 2].color = color;
                m_vertices[6 * i + 3].color = color;
                m_vertices[6 * i + 4].color = color;
                m_vertices[6 * i + 5].color = color;
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
    glDrawElements (GL_TRIANGLES, m_numIdxs, GL_UNSIGNED_INT, 0);

    // reset
    m_program->disableAttributeArray (vertexLocation);
    m_program->disableAttributeArray (colorLocation);
    m_indexBuf.release();
    m_arrayBuf.release();

    m_program->release();
}


void Depth3dSamsungS7::initGeometry ()
{
    Renderable::initGeometry();
    m_vertices = new VertexData[m_currentDataset->height * m_currentDataset->width * 6];
    m_indices = new GLuint[m_currentDataset->height * m_currentDataset->width * 6];

    m_numVtxs = m_currentDataset->height * m_currentDataset->width * 6;
    m_numIdxs = m_currentDataset->height * m_currentDataset->width * 6;

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
