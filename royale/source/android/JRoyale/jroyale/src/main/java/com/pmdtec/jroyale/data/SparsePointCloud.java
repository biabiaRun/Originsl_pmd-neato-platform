/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.data;

import java.util.*;

/**
 * The sparse point cloud gives XYZ and confidence for every valid
 * point.
 * It is given as an array of packed coordinate quadruplets (x,y,z,c)
 * as floating point values. The x, y and z coordinates are in meters.
 * The confidence (c) has a floating point value in [0.0, 1.0], where 1
 * corresponds to full confidence.
 */
public class SparsePointCloud
{
    private static final float[] DUMMY_POINTS = new float[0];

    /**
     * timestamp for the frame
     */
    public long timestamp;

    /**
     * stream which produced the data
     */
    public int streamId;

    /**
     * the number of valid points
     */
    public long numPoints;

    /**
     * XYZ and confidence for every valid point
     */
    public float[] xyzcPoints;

    /**
     * Dummy Constructor
     * <p>
     * fills a SparsePointCloud object with dummy values
     */
    public SparsePointCloud()
    {
        this(0L, 0, 0, DUMMY_POINTS);
    }

    /**
     * Deep Copy Constructor
     *
     * @param sparsePointCloud the SparsePointCloud to (deep-)copy
     */
    public SparsePointCloud(SparsePointCloud sparsePointCloud)
    {
        this(
                sparsePointCloud.timestamp,
                sparsePointCloud.streamId,
                sparsePointCloud.numPoints,
                sparsePointCloud.xyzcPoints
        );
    }

    /**
     * Initializing Constructor
     *
     * @param timestamp  the timestamp to set
     * @param streamId   the streamId to set
     * @param numPoints  the numPoints to set
     * @param xyzcPoints the xyzcPoints to set
     */
    public SparsePointCloud(long timestamp, int streamId, long numPoints, float[] xyzcPoints)
    {
        this.timestamp = timestamp;
        this.streamId = streamId;
        this.numPoints = numPoints;
        this.xyzcPoints = xyzcPoints;
    }

    @Override
    public String toString()
    {
        return "SparsePointCloud{" +
                "timestamp=" + timestamp +
                ", streamId=" + streamId +
                ", numPoints=" + numPoints +
                ", xyzcPoints=" + Arrays.toString(xyzcPoints) +
                '}';
    }
}
