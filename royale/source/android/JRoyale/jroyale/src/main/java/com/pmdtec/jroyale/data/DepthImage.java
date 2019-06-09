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
 * The depth image represents the depth and confidence for every pixel.
 * The least significant 13 bits are the depth (z value along the optical axis) in
 * millimeters. 0 stands for invalid measurement / no data.
 * The most significant 3 bits correspond to a confidence value.
 */
public class DepthImage
{
    private static final int[] DUMMY_DATA = new int[0];

    /**
     * timestamp for the frame
     */
    public long timestamp;

    /**
     * stream which produced the data
     */
    public int streamId;

    /**
     * width of depth image
     */
    public int width;

    /**
     * height of depth image
     */
    public int height;

    /**
     * depth and confidence for the pixel
     */
    public int[] data;

    /**
     * Dummy Constructor
     * <p>
     * fills a DepthImage object with dummy values
     */
    public DepthImage()
    {
        this(0L, 0, 0, 0, DUMMY_DATA);
    }

    /**
     * Deep Copy Constructor
     *
     * @param depthImage the DepthImage to (deep-)copy
     */
    public DepthImage(DepthImage depthImage)
    {
        this(
                depthImage.timestamp,
                depthImage.streamId,
                depthImage.width,
                depthImage.height,
                depthImage.data.clone()
        );
    }

    /**
     * Initializing Constructor
     *
     * @param timestamp the timestamp to set
     * @param streamId  the streamId to set
     * @param width     the width to set
     * @param height    the height to set
     * @param data      the data to set
     */
    public DepthImage(long timestamp, int streamId, int width, int height, int[] data)
    {
        this.timestamp = timestamp;
        this.streamId = streamId;
        this.width = width;
        this.height = height;
        this.data = data;
    }

    @Override
    public String toString()
    {
        return "DepthImage{" +
                "timestamp=" + timestamp +
                ", streamId=" + streamId +
                ", width=" + width +
                ", height=" + height +
                ", data=" + Arrays.toString(data) +
                '}';
    }
}
