/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.data;

import android.util.*;

import java.util.*;

/**
 * This container stores the lens parameters from the camera module
 */
public class LensParameters
{
    private static final float[] DUMMY_DISTORTION_RADIAL = new float[0];

    /**
     * cx/cy
     */
    public Pair<Float, Float> principalPoint;

    /**
     * fx/fy
     */
    public Pair<Float, Float> focalLength;

    /**
     * p1/p2
     */
    public Pair<Float, Float> distortionTangential;

    /**
     * k1/k2/k3
     */
    public float[] distortionRadial;

    /**
     * Dummy Constructor
     * <p>
     * fills a LensParameters object with dummy values
     */
    public LensParameters()
    {
        this(new Pair<>(0.0F, 0.0F), new Pair<>(0.0F, 0.0F), new Pair<>(0.0F, 0.0F), DUMMY_DISTORTION_RADIAL);
    }

    /**
     * Deep Copy Constructor
     *
     * @param lensParameters the LensParameters to (deep-)copy
     */
    public LensParameters(LensParameters lensParameters)
    {
        this(
                lensParameters.principalPoint,
                lensParameters.focalLength,
                lensParameters.distortionTangential,
                lensParameters.distortionRadial
        );
    }

    /**
     * Initializing Constructor
     *
     * @param principalPoint       the principalPoint to set
     * @param focalLength          the focalLength to set
     * @param distortionTangential the distortionTangential to set
     * @param distortionRadial     the distortionRadial to set
     */
    public LensParameters(Pair<Float, Float> principalPoint, Pair<Float, Float> focalLength, Pair<Float, Float> distortionTangential, float[] distortionRadial)
    {
        this.principalPoint = principalPoint;
        this.focalLength = focalLength;
        this.distortionTangential = distortionTangential;
        this.distortionRadial = distortionRadial;
    }

    @Override
    public String toString()
    {
        return "LensParameters{" +
                "principalPoint=" + principalPoint +
                ", focalLength=" + focalLength +
                ", distortionTangential=" + distortionTangential +
                ", distortionRadial=" + Arrays.toString(distortionRadial) +
                '}';
    }
}
