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
 * Infrared image with 8Bit mono information for every pixel
 */
public class IRImage
{
    private static final byte[] DUMMY_DATA = new byte[0];

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
     * 8Bit mono IR image
     */
    public byte[] data;

    /**
     * Dummy Constructor
     * <p>
     * fills a IRImage object with dummy values
     */
    public IRImage()
    {
        this(0L, 0, 0, 0, DUMMY_DATA);
    }

    /**
     * Deep Copy Constructor
     *
     * @param iRImage the IRImage to (deep-)copy
     */
    public IRImage(IRImage iRImage)
    {
        this(
                iRImage.timestamp,
                iRImage.streamId,
                iRImage.width,
                iRImage.height,
                iRImage.data
        );
    }

    /**
     * Initializing Constructor
     *
     * @param timestamp the timestamp to set
     * @param streamId the streamId to set
     * @param width the width to set
     * @param height the height to set
     * @param data the data to set
     */
    public IRImage(long timestamp, int streamId, int width, int height, byte[] data)
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
        return "IRImage{" +
                "timestamp=" + timestamp +
                ", streamId=" + streamId +
                ", width=" + width +
                ", height=" + height +
                ", data=" + Arrays.toString(data) +
                '}';
    }
}
