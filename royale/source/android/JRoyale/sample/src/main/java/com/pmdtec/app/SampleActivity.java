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
import com.pmdtec.jroyale.listener.*;

import android.app.*;
import android.os.*;
import android.util.Log;
import android.widget.*;

/**
 * the most simple sample for receiving depth
 * data from a royale camera and displaying it on the screen
 */
public class SampleActivity extends Activity
{
    private static final String TAG = "com.pmdtec.app.SampleActivity";

    private CameraDevice mCameraDevice;

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
        CameraManager cameraManager = CameraManager.createRoyaleCameraManager();

        // the create camera callback will be invoked
        // when a camera was created successfully
        // or if an error occurred while the camera
        // was tried to be created
        CameraCreationCallback creationCallback = new MyCameraCreationCallback();

        // we tell the camera manager that we want a camera to be created
        // and register a callback for the camera to be created
        //
        // this method assumes that there
        // is one physical camera to be created
        // (the first royale camera will be used)
        cameraManager.createCamera(this, creationCallback);

        // the camera manager can be deleted here
        // in this case we will not call it anymore
    }

    /**
     * A simple implementation of the {@link com.pmdtec.jroyale.CameraCreationCallback}
     */
    private class MyCameraCreationCallback implements CameraCreationCallback
    {
        @Override
        public void onOpen(CameraDevice cameraDevice)
        {
            try
            {
                // The initialize method needs to be called before
                // you can use all camera functionality.
                // On the other hand cameraDevice setExternalTrigger must be
                // called before the initialization to work.
                cameraDevice.initialize();

                // The DepthDataListener as every listener interface by jroyale
                // is a functional interface and therefor allows us to use lambdas
                // as listener.
                // In this case we use the data given by the camera device to
                // generate a bitmap representing the depth data and display it.
                DepthDataListener depthDataListener = data -> runOnUiThread(() ->
                {
                    // Get the ui component to display the data.
                    ImageView imageView = findViewById(id.camera_image_view);

                    // Display the bitmap to the ui component.
                    imageView.setImageBitmap(data.toBitmap());
                });

                // Register the listener to the camera device.
                cameraDevice.registerDataListener(depthDataListener);

                // Store a camera device reference in the Activity object
                // so we can handle events from the Activity
                //
                // noinspection SyntheticAccessorCall
                mCameraDevice = cameraDevice;

                // start to capture images
                cameraDevice.startCapture();
            }
            catch (CameraException e)
            {
                String message = e.getMessage();
                Log.e(TAG, message);
            }
        }

        @Override
        public void onError(CameraCreationError cameraCreationError)
        {
            String msg = "camera failed to be created (" + cameraCreationError + ')';
            Log.e(TAG, msg);
        }
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();

        if (null != mCameraDevice)
        {
            try
            {
                mCameraDevice.stopCapture();
            }
            catch (CameraException e)
            {
                String message = e.getMessage();
                Log.e(TAG, message);
            }
        }
    }

    @Override
    protected void onPause()
    {
        super.onPause();

        if (null != mCameraDevice)
        {
            try
            {
                mCameraDevice.stopCapture();
            }
            catch (CameraException e)
            {
                String message = e.getMessage();
                Log.e(TAG, message);
            }
        }
    }

    @Override
    protected void onResume()
    {
        super.onResume();

        if (null != mCameraDevice)
        {
            try
            {
                mCameraDevice.startCapture();
            }
            catch (CameraException e)
            {
                String message = e.getMessage();
                Log.e(TAG, message);
            }
        }
    }
}
