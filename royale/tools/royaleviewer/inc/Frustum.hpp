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

#include "Renderable.hpp"

QT_FORWARD_DECLARE_STRUCT (VertexData);

class Frustum : public Renderable
{
public:
    Frustum();
    virtual ~Frustum();
    virtual bool initGLWithFov (const float fovx, const float fovy);
    virtual void render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform) override;

    void updateFov (const float fovx, const float fovy);

protected:
    using Renderable::initGeometry;
    virtual void initGeometry (const float fovx, const float fovy);

    VertexData  *m_vertices;
    GLushort    *m_indices;

};
