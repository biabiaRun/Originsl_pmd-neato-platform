/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.data;

/**
 * Encapsulates a 3D point in object space, with coordinates in meters.  In addition to the
 * X/Y/Z coordinate each point also includes a gray value, a noise standard deviation, and a
 * depth confidence value.
 */
public class DepthPoint
{
    /**
     * X coordinate [meters]
     */
    public float x;

    /**
     * Y coordinate [meters]
     */
    public float y;

    /**
     * Z coordinate [meters]
     */
    public float z;

    /**
     * noise value [meters]
     */
    public float noise;

    /**
     * 16-bit gray value
     */
    public int grayValue;

    /**
     * value from 0 (invalid) to 255 (full confidence)
     */
    public int depthConfidence;

    /**
     * Dummy Constructor
     * <p>
     * fills a depthData object with dummy values
     */
    public DepthPoint()
    {
        this(0.0F, 0.0F, 0.0F, 0.0F, 0, 0);
    }

    /**
     * Deep Copy Constructor
     *
     * @param depthPoint the depthPoint to (deep-)copy
     */
    public DepthPoint(DepthPoint depthPoint)
    {
        this(
                depthPoint.x,
                depthPoint.y,
                depthPoint.z,
                depthPoint.noise,
                depthPoint.grayValue,
                depthPoint.depthConfidence);
    }

    /**
     * Initializing Constructor
     *
     * @param x               the X Coordinate to set
     * @param y               the Y Coordinate to set
     * @param z               the Z Coordinate to set
     * @param noise           the noise value to set
     * @param grayValue       the gray value to set
     * @param depthConfidence the depthConfidence to set
     */
    public DepthPoint(float x, float y, float z, float noise, int grayValue, int depthConfidence)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.noise = noise;
        this.grayValue = grayValue;
        this.depthConfidence = depthConfidence;
    }

    @Override
    public String toString()
    {
        return "DepthPoint{ x=" + x +
                ", y=" + y +
                ", z=" + z +
                ", noise=" + noise +
                ", grayValue=" + grayValue +
                ", depthConfidence=" + depthConfidence +
                " }";
    }
}
