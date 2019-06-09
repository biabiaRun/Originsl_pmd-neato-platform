/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale;

/**
 * This enum defines the access level.
 */
public enum CameraAccessLevel
{
    /**
     * Level 1 access provides depth data using standard, known-working configurations
     */
    L1,

    /**
     * Level 2 access provides raw data, e.g. for custom processing pipelines
     */
    L2,

    /**
     * Level 3 access enables you to overwrite exposure limits
     */
    L3,

    /**
     * Level 4 access is for bringing up new camera modules
     */
    L4
}
