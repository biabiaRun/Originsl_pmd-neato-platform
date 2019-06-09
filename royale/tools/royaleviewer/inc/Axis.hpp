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

class Axis : public Renderable
{
public:
    Axis();
    virtual ~Axis();

    virtual bool initGL () override;
    virtual void render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform) override;

protected:
    virtual void initGeometry() override;
    VertexData *m_vertices;
    GLushort *m_indices;
};
