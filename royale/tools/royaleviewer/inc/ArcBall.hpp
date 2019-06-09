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

#include <stdio.h>
#include <assert.h>
#include "Math0.hpp"

class ArcBall
{
private:
    float Epsilon;
    vec3f mouseDownLocation;
    vec3f dragVector;
    float width;
    float height;
    QMatrix4x4 m_qrotPrev;
    QMatrix4x4 m_qrotCurr;
    vec3f m_lastMouseMoveLocation;
    vec3f m_lastMouseDownLocation;

public:
    ArcBall (float width, float height)
    {
        Epsilon = 1.0e-5f;
        setBounds (width, height);
        mouseDown (vec3f (0, 0, 0));
        mouseMove (vec3f (0, 0, 0));
    }

    void setBounds (float width, float height)
    {
        assert (width > 1.0f && height > 1.0f);
        this->width = 1.0f / ( (width - 1.0f) * 0.5f);
        this->height = 1.0f / ( (height - 1.0f) * 0.5f);
    }

    float getBoundWidth()
    {
        return width;
    }

    float getBoundHeight()
    {
        return height;
    }


    const QMatrix4x4 &rotationMatrix() const
    {
        return m_qrotCurr;
    }

    void reset (const QMatrix4x4 &mat)
    {
        m_qrotCurr = m_qrotPrev = mat;
    }

    void reset()
    {
        m_qrotCurr.setToIdentity();
        m_qrotPrev.setToIdentity();
    }

    vec3f mapToSphere (const vec3f &screenPoint) const
    {
        vec3f vec;
        vec3f tmp = screenPoint;
        tmp.x = tmp.x * width - 1.0f;
        tmp.y = 1.0f - tmp.y * height;
        float l = tmp.x * tmp.x + tmp.y * tmp.y;
        if (l > 1.0f)
        {
            float n = 1.0f / sqrtf (l);
            vec.x = tmp.x * n;
            vec.y = tmp.y * n;
            vec.z = 0.0f;
        }
        else
        {
            vec.x = tmp.x;
            vec.y = tmp.y;
            vec.z = sqrtf (1.0f - l);
        }
        return vec;
    }

    void mouseDown (const vec3f &mouseLocation)
    {
        m_lastMouseDownLocation = mouseLocation;
        mouseDownLocation = mapToSphere (mouseLocation);
        m_qrotPrev = m_qrotCurr;
    }

    const vec3f &lastMouseDownLocation()
    {
        return m_lastMouseDownLocation;
    }
    const vec3f &lastMouseMoveLocation()
    {
        return m_lastMouseMoveLocation;
    }

    void mouseMove (const vec3f &mousePoint)
    {
        m_lastMouseMoveLocation = mousePoint;
        quaternion q;
        dragVector = mapToSphere (mousePoint);
        vec3f perp = vec3f::crossProduct (mouseDownLocation, dragVector);
        if (perp.length () > Epsilon)
        {
            q.set (vec3f::dotProduct (mouseDownLocation, dragVector), perp.x, perp.y, perp.z);
        }
        else
        {
            q.reset();
        }
        q.toMatrix (m_qrotCurr);
        m_qrotCurr *= m_qrotPrev;
    }
};
