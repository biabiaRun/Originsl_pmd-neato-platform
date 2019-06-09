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
#include <math.h>
#include <QtGui/QMatrix4x4>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline const float clipAbove (const float &t, const float &v)
{
    return (v > t) ? t : v;
}

inline const float clipBelow (const float &t, const float &v)
{
    return (v < t) ? t : v;
}

class quaternion
{
public:
    float w;
    float x;
    float y;
    float z;

    quaternion() : w (1.0f), x (0.0f), y (0.0f), z (0.0f) {}

    void reset()
    {
        w = x = y = z = 0.0f;
    }

    void set (float w, float x, float y, float z)
    {
        this->w = w;
        this->x = x;
        this->y = y;
        this->z = z;
    }

    void toMatrix (QMatrix4x4 &mat)
    {
        float n = this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;
        float s = n > 0.0f ? 2.0f / n : 0.0f;
        float xs = this->x * s;
        float ys = this->y * s;
        float zs = this->z * s;
        float wx = this->w * xs;
        float wy = this->w * ys;
        float wz = this->w * zs;
        float xx = this->x * xs;
        float xy = this->x * ys;
        float xz = this->x * zs;
        float yy = this->y * ys;
        float yz = this->y * zs;
        float zz = this->z * zs;
        mat (0, 0) = 1.0f - (yy + zz);
        mat (0, 1) = xy - wz;
        mat (0, 2) = xz + wy;
        mat (0, 3) = 0.0f;
        mat (1, 0) = xy + wz;
        mat (1, 1) = 1.0f - (xx + zz);
        mat (1, 2) = yz - wx;
        mat (1, 3) = 0.0f;
        mat (2, 0) = xz - wy;
        mat (2, 1) = yz + wx;
        mat (2, 2) = 1.0f - (xx + yy);
        mat (2, 3) = 0.0f;
        mat (3, 0) = 0.0f;
        mat (3, 1) = 0.0f;
        mat (3, 2) = 0.0f;
        mat (3, 3) = 1.0f;
    }

};

class vec3f
{
public:
    vec3f() : x (0.0f), y (0.0f), z (0.0f) {}
    vec3f (float x, float y, float z) : x (x), y (y), z (z) {}
    explicit vec3f (float v[3]) : x (v[0]), y (v[1]), z (v[2]) {}
    vec3f (const vec3f &v) : x (v.x), y (v.y), z (v.z) {}

    static vec3f crossProduct (const vec3f &a, const vec3f &b)
    {
        vec3f v;
        v.x = a.y * b.z - a.z * b.y;
        v.y = a.z * b.x - a.x * b.z;
        v.z = a.x * b.y - a.y * b.x;
        return v;
    }

    static float dotProduct (const vec3f &a, const vec3f &b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    float length()
    {
        return sqrtf (vec3f::dotProduct (*this, *this));
    }

    vec3f &operator= (const vec3f &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

public:
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };
        struct
        {
            float u;
            float v;
            float w;
        };
        float value[3];
    };
};

class Matrix4x4Helpers
{
public:
    static void decomposeRotation (const QMatrix4x4 &composedRotationMatrix, float &xAngle, float &yAngle, float &zAngle)
    {
        float floatPI = static_cast<float> (M_PI);
        if (composedRotationMatrix (1, 0) > 0.998f)
        {
            yAngle = atan2f (composedRotationMatrix (0, 2), composedRotationMatrix (2, 2)) * 180.0f / floatPI;
            zAngle = floatPI / 2.0f * 180.0f / floatPI;
            xAngle = 0.0f * 180.0f / floatPI;
        }
        else if (composedRotationMatrix (0, 0) < -0.998f)
        {
            yAngle = atan2f (composedRotationMatrix (0, 2), composedRotationMatrix (2, 2)) * 180.0f / floatPI;
            zAngle = -floatPI / 2.0f * 180.0f / floatPI;
            xAngle = 0.0f * 180.0f / floatPI;
        }
        else
        {
            yAngle = atan2f (-composedRotationMatrix (2, 0), composedRotationMatrix (0, 0)) * 180.0f / floatPI;
            zAngle = asinf (composedRotationMatrix (1, 0)) * 180.0f / floatPI;
            xAngle = atan2f (-composedRotationMatrix (1, 2), composedRotationMatrix (1, 1)) * 180.0f / floatPI;
        }
    }
};
