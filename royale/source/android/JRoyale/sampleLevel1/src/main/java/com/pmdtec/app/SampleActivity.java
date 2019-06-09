/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.app;

import com.pmdtec.app.R.*;
import com.pmdtec.jroyale.*;

import android.app.*;
import android.os.*;

/**
 * the most simple sample for receiving depth
 * data from a royale camera and displaying it on the screen
 */
public class SampleActivity extends Activity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(layout.activity_sample);
        requestCamera();
    }

    /**
     * This method will use the
     * {@link com.pmdtec.jroyale.RoyaleCameraManager}
     * to register a camera creation callback
     * and request a camera to be created
     */
    private void requestCamera()
    {
        // the camera manager is needed to create a camera
        // and set some settings (coming soon)
        //
        CameraManager cameraManager = CameraManager.createRoyaleCameraManager();

        // the create camera callback will be invoked
        // when a camera was created successfully
        // or if an error occurred while the camera
        // was tried to be created
        //
        CameraCreationCallback creationCallback = new SampleCameraCreationCallback(this);

        // we tell the camera manager that we want a camera to be created
        // and register a callback for the camera to be created
        //
        // this method assumes that there
        // is one physical camera to be created
        // (the first royale camera will be used)
        //
        cameraManager.createCamera(this, creationCallback);

        // the camera manager can be deleted here
        // in this case we will not call it anymore
    }
}
