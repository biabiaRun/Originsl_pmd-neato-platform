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
 * the CameraException is an Alias for the royale STATUS.
 * When royale returns a value not equal to null, an CameraException is thrown.
 */
public class CameraException extends RuntimeException
{
    private static final long serialVersionUID = 8704497657588879006L;

    /**
     * initializing constructor
     *
     * @param message the message to set
     * @see java.lang.RuntimeException#RuntimeException(String)
     */
    public CameraException(String message)
    {
        super(message);
    }
}
