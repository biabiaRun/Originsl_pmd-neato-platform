/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <algorithm>

#include "ColorHelper.hpp"

ColorHelper::ColorHelper()
{
    m_minDist = M_DEFAULT_MIN_DIST;
    m_maxDist = M_DEFAULT_MAX_DIST;
    calcSpanDist();

    m_minVal = M_DEFAULT_MIN_VAL;
    m_maxVal = M_DEFAULT_MAX_VAL;
    calcSpanVal();

    for (auto i = 0u; i < M_COLOR_LOOKUP_SIZE; ++i)
    {
        auto h = static_cast<uint16_t> (i);

        HsvColor tempHsv;
        tempHsv.h = h;
        tempHsv.s = 255;
        tempHsv.v = 255;

        m_colorLookup[i] = HsvToRgb (tempHsv);
    }

    for (auto i = 0u; i < M_GRAY_LOOKUP_SIZE; ++i)
    {
        auto grayVal = static_cast<uint16_t> (i);

        RgbColor grayColor;
        grayColor.r = grayVal;
        grayColor.g = grayVal;
        grayColor.b = grayVal;

        m_grayLookup[i] = grayColor;
    }
}

RgbColor ColorHelper::HsvToRgb (const HsvColor &hsv)
{
    RgbColor rgb;
    uint16_t region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = static_cast<uint16_t> ( (hsv.h % 43) * 6);

    p = static_cast<uint16_t> ( (hsv.v * (255 - hsv.s)) >> 8);
    q = static_cast<uint16_t> ( (hsv.v * (255 - ( (hsv.s * remainder) >> 8))) >> 8);
    t = static_cast<uint16_t> ( (hsv.v * (255 - ( (hsv.s * (255 - remainder)) >> 8))) >> 8);

    switch (region)
    {
        case 0:
            rgb.r = hsv.v;
            rgb.g = t;
            rgb.b = p;
            break;
        case 1:
            rgb.r = q;
            rgb.g = hsv.v;
            rgb.b = p;
            break;
        case 2:
            rgb.r = p;
            rgb.g = hsv.v;
            rgb.b = t;
            break;
        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v;
            rgb.g = p;
            rgb.b = q;
            break;
    }

    return rgb;
}

const RgbColor &ColorHelper::getGrayColor (const uint16_t val)
{
    uint16_t clampedVal = std::min (m_maxVal, val);
    clampedVal = std::max (m_minVal, clampedVal);
    int index = std::min<uint16_t> (M_GRAY_LOOKUP_SIZE - 1, static_cast<uint16_t> ( (M_GRAY_LOOKUP_SIZE - 1) * (clampedVal - m_minVal) / m_spanVal));

    if (index < 0)
    {
        index = 0;
    }

    return m_grayLookup[index];
}

const RgbColor &ColorHelper::getColor (const float dist)
{
    float clampedDist = std::min (m_maxDist, dist);
    clampedDist = std::max (m_minDist, clampedDist);
    int index = std::min<uint16_t> (M_COLOR_LOOKUP_SIZE - 1, static_cast<uint16_t> (static_cast<float> (M_COLOR_LOOKUP_SIZE - 1) * (clampedDist - m_minDist) / m_spanDist));

    if (index < 0)
    {
        index = 0;
    }

    return m_colorLookup[index];
}

void ColorHelper::setMinDist (float dist)
{
    if (dist < 0)
    {
        dist = 0;
    }

    auto minMaxDist = dist + M_MIN_DISTANCE_SPAN;
    if (minMaxDist > m_maxDist)
    {
        m_maxDist = minMaxDist;
    }

    m_minDist = dist;

    calcSpanDist();
}

float ColorHelper::getMinDist()
{
    return m_minDist;
}

void ColorHelper::setMaxDist (float dist)
{
    if (dist < M_MIN_DISTANCE_SPAN)
    {
        dist = M_MIN_DISTANCE_SPAN;
    }

    auto maxMinDist = dist - M_MIN_DISTANCE_SPAN;
    if (maxMinDist < m_minDist)
    {
        m_minDist = maxMinDist;
    }

    m_maxDist = dist;
    calcSpanDist();
}

float ColorHelper::getMaxDist()
{
    return m_maxDist;
}

void ColorHelper::calcSpanDist()
{
    m_spanDist = m_maxDist - m_minDist;
}

void ColorHelper::setMinVal (uint16_t val)
{
    if (val < 0)
    {
        val = 0;
    }

    auto minMaxVal = val + M_MIN_VAL_SPAN;
    if (minMaxVal > m_maxVal)
    {
        m_maxVal = static_cast<uint16_t> (minMaxVal);
    }

    m_minVal = val;
    calcSpanVal();
}

uint16_t ColorHelper::getMinVal()
{
    return m_minVal;
}

void ColorHelper::setMaxVal (uint16_t val)
{
    if (val < M_MIN_VAL_SPAN)
    {
        val = M_MIN_VAL_SPAN;
    }

    auto maxMinVal = val - M_MIN_VAL_SPAN;
    if (maxMinVal < m_minVal)
    {
        m_minVal = static_cast<uint16_t> (maxMinVal);
    }

    m_maxVal = val;
    calcSpanVal();
}

uint16_t ColorHelper::getMaxVal()
{
    return m_maxVal;
}

void ColorHelper::calcSpanVal()
{
    m_spanVal = static_cast<uint16_t> (m_maxVal - m_minVal);
}
