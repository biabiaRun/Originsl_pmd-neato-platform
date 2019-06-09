/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.data;

import android.graphics.*;
import android.graphics.Bitmap.*;

import java.util.*;

/**
 * This structure defines the depth data which is delivered through the callback.
 * This data comprises a dense 3D point cloud with the size of the depth image (width, height).
 * The points vector encodes an array (row-based) with the size of (width x height). Based
 * on the `depthConfidence`, the user can decide to use the 3D point or not.
 * The point cloud uses a right handed coordinate system (x -> right, y -> down,
 * z -> in viewing direction).
 * <p>
 * Although the points are provided as a (width * height) array and are arranged in a grid,
 * treating them as simple square pixels will provide a distorted image, because they are not
 * necessarily in a rectilinear projection; it is more likely that the camera would have a
 * wide-angle or even fish-eye lens.  Each individual DepthPoint provides x and y coordinates
 * in addition to the z cooordinate, these values in the individual DepthPoints will match the
 * lens of the camera.
 */
public class DepthData
{
    private static final long[] DUMMY_EXPOSURE_TIMES = new long[0];
    private static final DepthPoint[] DUMMY_POINTS = new DepthPoint[0];

    /**
     * version number of the data format
     */
    public int version;

    /**
     * timestamp in microseconds precision (time since epoch 1970)
     */
    public long timeStamp;

    /**
     * stream which produced the data
     */
    public long streamId;

    /**
     * width of depth image
     */
    public int width;

    /**
     * height of depth image
     */
    public int height;

    /**
     * exposureTimes retrieved from CapturedUseCase
     */
    public long[] exposureTimes;

    /**
     * list of points
     */
    public DepthPoint[] points;

    /**
     * Dummy Constructor
     * <p>
     * fills a depthData object with dummy values
     */
    public DepthData()
    {
        this(0, 0L, 0L, 0, 0, DUMMY_EXPOSURE_TIMES, DUMMY_POINTS);
    }

    /**
     * Deep Copy Constructor
     *
     * @param depthData the depthData to (deep-)copy
     */
    public DepthData(DepthData depthData)
    {
        this(
                depthData.version,
                depthData.timeStamp,
                depthData.streamId,
                depthData.width,
                depthData.height,
                new long[depthData.exposureTimes.length],
                new DepthPoint[depthData.points.length]
        );

        System.arraycopy(depthData.exposureTimes, 0, exposureTimes, 0, depthData.exposureTimes.length);
        System.arraycopy(depthData.points, 0, points, 0, depthData.points.length);
    }

    /**
     * Initializing Constructor
     *
     * @param version       the version to set
     * @param timeStamp     the timeStamp to set
     * @param streamId      the streamId to set
     * @param width         the width to set
     * @param height        the height to set
     * @param exposureTimes the exposureTimes to set (no deep copy!!!)
     * @param points        the points to set (no deep copy!!!)
     */
    public DepthData(int version,
                     long timeStamp,
                     long streamId,
                     int width,
                     int height,
                     long[] exposureTimes,
                     DepthPoint[] points)
    {
        this.version = version;
        this.timeStamp = timeStamp;
        this.streamId = streamId;
        this.width = width;
        this.height = height;
        this.exposureTimes = exposureTimes;
        this.points = points;
    }

    /**
     * @return a Bitmap representation of the object
     */
    public Bitmap toBitmap()
    {
        // this value describes the expected biggest
        // measured depth value at the moment [meters]
        //
        @SuppressWarnings("MagicNumber")
        float maxShownZ = 7.5F;

        // 360 is the max value for HUE in the HSV-Color range
        //
        @SuppressWarnings("MagicNumber")
        float maxHUE = 360.0F;

        // provides the heap for the hsv color to be stored
        //
        // colorBlueprint[1] and colorBlueprint[2]
        // should be handled as const
        //
        // colorBlueprint[0] describes the distance
        // of an object (between 0.0F and 360.0F)
        //
        float[] colorBlueprint = {0.0F, 1.0F, 1.0F};

        // the bitmap data represented as an int array
        // where the int values describe an rgb color
        //
        int[] buffer = new int[points.length];

        // iterate each point of the bitmap data and
        // set its value to a color matching to its distance
        //
        int length = points.length;
        for (int i = 0; i < length; i++)
        {
            // if the pixel is not valid
            // set the color for this pixel to BLACK
            // (like the royale viewer)
            //
            if (0.0F == points[i].depthConfidence)
            {
                buffer[i] = Color.BLACK;
            }
            else
            {
                float hue = (points[i].z / maxShownZ) * maxHUE;

                // catch the case that points[i].z is less than one
                // or bigger than the expected biggest measured depth
                // value
                //
                if (maxHUE < hue)
                {
                    hue = maxHUE;
                }
                else if (0.0F > hue)
                {
                    hue = 0.0F;
                }

                colorBlueprint[0] = hue;

                // HSV float array to RGB integer
                buffer[i] = Color.HSVToColor(colorBlueprint);
            }
        }

        Bitmap bmp = Bitmap.createBitmap(width, height, Config.ARGB_8888);
        bmp.setPixels(buffer, 0, width, 0, 0, width, height);
        return bmp;
    }

    @Override
    public String toString()
    {
        return "DepthData{ version=" + version +
                ", timeStamp=" + timeStamp +
                ", streamId=" + streamId +
                ", width=" + width +
                ", height=" + height +
                ", exposureTimes=" + Arrays.toString(exposureTimes) +
                ", points=" + Arrays.toString(points) +
                " }";
    }
}
