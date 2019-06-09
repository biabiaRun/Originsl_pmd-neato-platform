/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <stdint.h>

/**
 * RGB color model
 *
 * Each parameter (r: red, g: green, b: blue) defines the intensity of the color as an integer between 0 and 255
 */
typedef struct RgbColor
{
    /**
     * The r value defines the intensity of the red color as an integer between 0 and 255
     */
    uint16_t r;

    /**
     * The g value defines the intensity of the green color as an integer between 0 and 255
     */
    uint16_t g;

    /**
     * The b value defines the intensity of the blue color as an integer between 0 and 255
     */
    uint16_t b;
} RgbColor;

// The value used for the SBI flag
static const uint32_t FLAGS_SBI_ON = 0x100u;

namespace
{
    /**
     * HSV color model
     */
    typedef struct HsvColor
    {
        /**
         * The h value defines the intensity of the hue as an integer between 0 and 255.
         * By default the hue value is described as an integer between 0 and 360.
         * to convert the h value to the real hue multiply it with 1.41176:
         * auto hue = static_cast<float> (h) * 1.41176;
         */
        uint16_t h;

        /**
         * The s value defines the intensity of the saturation as an integer between 0 and 255.
         * By default the saturation value is described as a float between 0 and 1.
         * to convert the s value to the real saturation multiply it with 0.00392157:
         * auto saturation = static_cast<float> (s) * 0.00392157;
         */
        uint16_t s;

        /**
         * The v value defines the intensity of the value as an integer between 0 and 255.
         * By default the value value is described as a float between 0 and 1.
         * to convert the v value to the real value multiply it with 0.00392157:
         * auto value = static_cast<float> (v) * 0.00392157;
         */
        uint16_t v;
    } HsvColor;
}

/**
 * The ColorHelper is a tool to get a color associated with a specific distance.
 * The Color returned depends on the min and max value set.
 *
 * The ColorHelper differentiates between two kinds of colors.
 * The default colors and the gray colors.
 *
 * To modify the default colors range use the setMinDist and setMaxDist methods.
 * To modify the gray colors range use the setMinVal and setMaxVal methods.
 */
class ColorHelper
{

public:
    ColorHelper();

    /**
     * returns a color associated with a given distance.
     */
    const RgbColor &getColor (const float dist);

    /**
     * returns a gray color associated with a given distance.
     */
    const RgbColor &getGrayColor (const uint16_t dist);

    /**
     * This should be the minimum for the
     * getColor method which uses the distances
     */
    void setMinDist (float dist);
    float getMinDist();

    /**
     * This should be the maximum for the
     * getColor method which uses the distances
     */
    void setMaxDist (float dist);
    float getMaxDist();

    /**
     * This should be the minimum for the
     * getGrayColor method which uses the distances
     */
    void setMinVal (uint16_t val);
    uint16_t getMinVal();

    /**
     * This should be the maximum for the
     * getGrayColor method which uses the distances
     */
    void setMaxVal (uint16_t val);
    uint16_t getMaxVal();

private:
    RgbColor HsvToRgb (const HsvColor &hsv);

    void calcSpanDist();
    void calcSpanVal();

    /**
     * The min distance span is used to prevent a division by zero exception in the getColor method.
     */
    const float M_MIN_DISTANCE_SPAN = 0.00001f;

    /**
     * The min val span is used to prevent an divided by zero exception in the getGrayColor method.
     */
    const uint16_t M_MIN_VAL_SPAN = 1;

    /**
     * The length of the color lookup array.
     *
     * The bigger this value the more precise the color given by the getColor method.
     *
     * It is chosen to match the needed precision for the RoyaleViewer.
     */
    static const uint16_t M_COLOR_LOOKUP_SIZE = 180;

    /**
     * The length of the gray color lookup array.
     *
     * The bigger this value the more precise the color given by the getGrayColor method.
     *
     * It is chosen to match the needed precision for the RoyaleViewer.
     */
    static const uint16_t M_GRAY_LOOKUP_SIZE = 256;

    RgbColor m_colorLookup[M_COLOR_LOOKUP_SIZE];
    RgbColor m_grayLookup[M_GRAY_LOOKUP_SIZE];

    /**
     * The default min dist is a hard coded value
     * that matches the needs of the RoyaleViewer.
     */
    const float M_DEFAULT_MIN_DIST = 0.1f;

    /**
     * The default max dist is a hard coded value
     * that matches the needs of the RoyaleViewer.
     */
    const float M_DEFAULT_MAX_DIST = 1.8f;

    /**
     * The default min val is a hard coded value
     * that matches the needs of the RoyaleViewer.
     */
    const uint16_t M_DEFAULT_MIN_VAL = 1;

    /**
     * The default max val is a hard coded value
     * that matches the needs of the RoyaleViewer.
     */
    const uint16_t M_DEFAULT_MAX_VAL = 100;

    float m_minDist;
    float m_maxDist;
    float m_spanDist;

    uint16_t m_minVal;
    uint16_t m_maxVal;
    uint16_t m_spanVal;
};
