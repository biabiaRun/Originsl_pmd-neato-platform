package com.pmdtec.app;

import android.content.*;
import android.os.*;
import android.util.*;

import com.pmdtec.jroyale.*;

import java.io.*;

/**
 * A simple implementation of the {@link com.pmdtec.jroyale.CameraCreationCallback}
 */
class SampleCameraCreationCallback implements CameraCreationCallback
{
    private static final String TAG = "com.pmdtec.app.SampleCameraCreationCallback";

    private Context context;

    SampleCameraCreationCallback(Context context)
    {
        this.context = context;
    }

    /**
     * Logs the toString of the given parameter using a maximum length of 64 as information
     *
     * @param objects the objects to logged
     */
    private static void logObjects(Object... objects)
    {
        for (Object object : objects)
        {
            String objString = object.toString();
            int objStringLength = objString.length();

            int maxLength = Math.min(64, objStringLength);
            objString = objString.substring(0, maxLength);

            Log.i(TAG, objString);
        }
    }

    @Override
    public void onOpen(CameraDevice cameraDevice)
    {
        // this method invokes all methods from the royal interface
        // that are level1 access:

        // notice that setExternalTrigger can only be called before
        // cameraDevice.initialize and can not be called after
        // cameraDevice.initialize anymore - this would throw an exception
        cameraDevice.setExternalTrigger(false);

        // initialize need to be called to control the camera
        // therefor it is recommended to call this method at the begin of the
        // onOpen method.
        // (after cameraDevice.setExternalTrigger if the external trigger needs to be set)
        cameraDevice.initialize();

        // in the following code we register all possible listeners to the camera
        // using the logObjects method as receiver for the data
        cameraDevice.registerDataListener(SampleCameraCreationCallback::logObjects);
        cameraDevice.registerDepthImageListener(SampleCameraCreationCallback::logObjects);
        cameraDevice.registerSparsePointCloudListener(SampleCameraCreationCallback::logObjects);
        cameraDevice.registerIRImageListener(SampleCameraCreationCallback::logObjects);
        cameraDevice.registerEventListener(SampleCameraCreationCallback::logObjects);
        cameraDevice.registerRecordListener(SampleCameraCreationCallback::logObjects);
        cameraDevice.registerExposureListener(SampleCameraCreationCallback::logObjects);

        cameraDevice.startCapture();

        // in the following code we print all information we can get from the camera
        logObjects("cameraDevice.getId = " + cameraDevice.getId());
        logObjects("cameraDevice.getCameraName = " + cameraDevice.getCameraName());
        logObjects("cameraDevice.getCameraInfo = " + cameraDevice.getCameraInfo());
        logObjects("cameraDevice.getUseCases = " + cameraDevice.getUseCases());
        logObjects("cameraDevice.getStreams = " + cameraDevice.getStreams());
        logObjects("cameraDevice.getCurrentUseCase = " + cameraDevice.getCurrentUseCase());
        logObjects("cameraDevice.getExposureMode = " + cameraDevice.getExposureMode());
        logObjects("cameraDevice.getExposureLimits = " + cameraDevice.getExposureLimits());
        logObjects("cameraDevice.getMaxSensorWidth = " + cameraDevice.getMaxSensorWidth());
        logObjects("cameraDevice.getMaxSensorHeight = " + cameraDevice.getMaxSensorHeight());
        logObjects("cameraDevice.getLensParameters = " + cameraDevice.getLensParameters());
        logObjects("cameraDevice.isConnected = " + cameraDevice.isConnected());
        logObjects("cameraDevice.isCalibrated = " + cameraDevice.isCalibrated());
        logObjects("cameraDevice.isCapturing = " + cameraDevice.isCapturing());
        logObjects("cameraDevice.getAccessLevel = " + cameraDevice.getAccessLevel());
        logObjects("cameraDevice.getFrameRate = " + cameraDevice.getFrameRate());
        logObjects("cameraDevice.getMaxFrameRate = " + cameraDevice.getMaxFrameRate());

        logObjects("cameraDevice.getNumberOfStreams = " + cameraDevice.getNumberOfStreams(cameraDevice.getUseCases().get(0)));

        // cameraDevice.getExposureMode(int streamId);
        // -> implicit called by cameraDevice.getExposureMode();
        // cameraDevice.getExposureLimits(int streamId);
        // -> implicit called by cameraDevice.getExposureLimits();

        // in the following code we set some parameters of the camera
        cameraDevice.setUseCase(cameraDevice.getUseCases().get(0));
        cameraDevice.setExposureTime(cameraDevice.getExposureLimits().first);
        // The sleep command is recommended because setting several parameters
        // can lead to a camera exception.
        // This is because the set methods of the camera device are non blocking.
        // So if you want to set a second parameter the camera can still be busy
        // setting the first parameter.
        sleepOneSecond();
        cameraDevice.setExposureMode(ExposureMode.MANUAL);
        sleepOneSecond();
        cameraDevice.setFrameRate(cameraDevice.getMaxFrameRate());

        File dir = context.getFilesDir();
        File file = new File(dir, "sampleRecordLevel1.rrf");
        cameraDevice.startRecording(file.getPath());

        // cameraDevice.setExposureTime(long exposureTime, int streamId);
        // -> implicit called by cameraDevice.setExposureTime(long exposureTime);
        // cameraDevice.setExposureMode(ExposureMode exposureMode, int streamId);
        // -> implicit called by cameraDevice.setExposureMode(ExposureMode exposureMode);

        // cameraDevice.startRecording(String fileName, long numberOfFrames, long frameSkip, long msSkip);
        // -> implicit called by startRecording(String fileName);
        // cameraDevice.startRecording(String fileName, long numberOfFrames, long frameSkip)
        // -> calling startRecording(String fileName);
        // cameraDevice.startRecording(String fileName, long numberOfFrames);
        // -> calling startRecording(String fileName);

        Handler handler = new Handler();
        handler.postDelayed(() -> {
            // we want the camera to stop capturing
            // after a fixed time in this example

            cameraDevice.stopRecording();
            cameraDevice.stopCapture();

            cameraDevice.unregisterDataListener();
            cameraDevice.unregisterDepthImageListener();
            cameraDevice.unregisterSparsePointCloudListener();
            cameraDevice.unregisterIRImageListener();
            cameraDevice.unregisterEventListener();
            cameraDevice.unregisterRecordListener();
            cameraDevice.unregisterExposureListener();

        }, 5000L);
    }

    @Override
    public void onError(CameraCreationError cameraCreationError)
    {
        String msg = "camera failed to be created (" + cameraCreationError + ')';
        Log.e(TAG, msg);
    }

    /**
     * try to sleep for 2000 Milliseconds or return if an error occurs
     */
    static void sleepOneSecond()
    {
        try
        {
            Thread.sleep(1000L);
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }
    }
}
