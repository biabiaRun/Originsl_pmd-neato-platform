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
 * a callback to handle the camera after it was created
 */
public interface CameraCreationCallback
{
    /**
     * @param cameraDevice the camera device
     */
    void onOpen(CameraDevice cameraDevice);

    /**
     * @param cameraCreationError the create camera cameraCreationError that occurred
     */
    void onError(CameraCreationError cameraCreationError);
}
