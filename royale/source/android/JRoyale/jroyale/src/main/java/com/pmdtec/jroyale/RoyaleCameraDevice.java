/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale;

import android.util.*;

import com.pmdtec.jroyale.data.*;
import com.pmdtec.jroyale.listener.*;

import java.util.*;

class RoyaleCameraDevice implements CameraDevice
{
    /*
     * notice that the m*Handle fields are only used by JNI to store object specific cpp-pointer
     */

    @SuppressWarnings("unused")
    private long mHandle;

    @SuppressWarnings("unused")
    private long mDepthDataListenerHandle;

    @SuppressWarnings("unused")
    private long mDepthImageListenerHandle;

    @SuppressWarnings("unused")
    private long mEventListenerHandle;

    @SuppressWarnings("unused")
    private long mExposureListenerHandle;

    @SuppressWarnings("unused")
    private long mIRImageListenerHandle;

    @SuppressWarnings("unused")
    private long mRecordStopListenerHandle;

    @SuppressWarnings("unused")
    private long mSparsePointCloudListenerHandle;

    @Override
    public native void initialize();

    @Override
    protected void finalize() throws Throwable
    {
        finalizeNative();
        super.finalize();
    }

    private native void finalizeNative();

    @Override
    public native String getId();

    @Override
    public native String getCameraName();

    @Override
    public native Map<String, String> getCameraInfo();

    @Override
    public native void setUseCase(String name);

    @Override
    public native List<String> getUseCases();

    @Override
    public native List<Integer> getStreams();

    @Override
    public native long getNumberOfStreams(String useCaseName);

    @Override
    public native String getCurrentUseCase();

    @Override
    public native void setExposureTime(long exposureTime, int streamId);

    @Override
    public native void setExposureMode(ExposureMode exposureMode, int streamId);

    @Override
    public native ExposureMode getExposureMode(int streamId);

    @Override
    public native Pair<Long, Long> getExposureLimits(int streamId);

    @Override
    public native void registerDataListener(DepthDataListener listener);

    @Override
    public native void unregisterDataListener();

    @Override
    public native void registerDepthImageListener(DepthImageListener listener);

    @Override
    public native void unregisterDepthImageListener();

    @Override
    public native void registerSparsePointCloudListener(SparsePointCloudListener listener);

    @Override
    public native void unregisterSparsePointCloudListener();

    @Override
    public native void registerIRImageListener(IRImageListener listener);

    @Override
    public native void unregisterIRImageListener();

    @Override
    public native void registerEventListener(CameraEventListener listener);

    @Override
    public native void unregisterEventListener();

    @Override
    public native void startCapture();

    @Override
    public native void stopCapture();

    @Override
    public native int getMaxSensorWidth();

    @Override
    public native int getMaxSensorHeight();

    @Override
    public native LensParameters getLensParameters();

    @Override
    public native boolean isConnected();

    @Override
    public native boolean isCalibrated();

    @Override
    public native boolean isCapturing();

    @Override
    public native CameraAccessLevel getAccessLevel();

    @Override
    public native void startRecording(String fileName, long numberOfFrames, long frameSkip, long msSkip);

    @Override
    public native void stopRecording();

    @Override
    public native void registerRecordListener(RecordStopListener listener);

    @Override
    public native void unregisterRecordListener();

    @Override
    public native void registerExposureListener(ExposureListener listener);

    @Override
    public native void unregisterExposureListener();

    @Override
    public native void setFrameRate(int frameRate);

    @Override
    public native int getFrameRate();

    @Override
    public native int getMaxFrameRate();

    @Override
    public native void setExternalTrigger(boolean useExternalTrigger);
}
