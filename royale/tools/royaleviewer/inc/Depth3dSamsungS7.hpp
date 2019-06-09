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

#include "Depth3d.hpp"

class Depth3dSamsungS7 : public Depth3d
{
public:
    Depth3dSamsungS7 (ColorHelper *colorHelper);
    virtual ~Depth3dSamsungS7();

    virtual void render (const QMatrix4x4 &mvMatrix, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelTransform) override;

protected:
    virtual void initGeometry () override;

};
