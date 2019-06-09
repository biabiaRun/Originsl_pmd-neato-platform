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

/**
 * This is the main interface for talking to the time-of-flight camera system (TOF).
 * Typically, an instance is created by the `CameraManager` which automatically detects a
 * connected module.
 * <p>
 * The support access levels can be activated by entering the correct code during creation.
 * <p>
 * Please refer to the provided examples (in the samples directory) for an overview on how to
 * use this class.
 */
public interface CameraDevice
{
    /**
     * LEVEL 1
     * Initializes the camera device and sets the first available use case.
     *
     * @throws CameraException with status CALIBRATION_DATA_ERROR, if the camera device has no calibration data (or data that
     *                         is incompatible with the processing, requiring a more recent version of Royale); this
     *                         device can not be used with Level 1. For bringing up a new module, level 2 access can
     *                         either access the hardware by closing this instance, creating a new instance, and calling
     *                         setCallbackData (CallbackData::Raw) before calling initialize() or by specifying different
     *                         calibration data by calling setCalibrationData (const royale::String &filename) before
     *                         calling initialize().<br>
     * @throws CameraException with status USECASE_NOT_SUPPORTED, if the camera device was successfully opened, but the
     *                         default use case could not be activated. This is only expected to happen when bringing
     *                         up a new module, so it's not expected at Level 1.<br>
     * @throws CameraException with status FILE_NOT_FOUND, may be returned when this ICameraDevice represents a recording.<br>
     * @throws CameraException with status DEVICE_ALREADY_INITIALIZED, if this is called more than once.
     */
    void initialize();

    /**
     * LEVEL 1
     *
     * @return the unique ID for the camera device
     * @throws CameraException if the id can not be received from the camera
     */
    String getId();

    /**
     * LEVEL 1
     *
     * @return the associated camera name as a string which is defined in the CoreConfig of each module.
     * @throws CameraException if the camera name can not be received from the camera
     */
    String getCameraName();

    /**
     * LEVEL 1
     * Retrieve further information for this specific camera.
     *
     * @return a map where the keys are depending on the used camera
     * @throws CameraException on royal error
     */
    Map<String, String> getCameraInfo();

    /**
     * LEVEL 1
     * Sets the use case for the camera. If the use case is supported by the connected
     * camera device SUCCESS will be returned. Changing the use case will also change the
     * processing parameters that are used (e.g. auto exposure)!
     * <p>
     * NOTICE: This function must not be called in the data callback - the behavior is
     * undefined. Call it from a different thread instead.
     *
     * @param name identifies the use case by an case sensitive string
     * @throws CameraException if the use case can not be set
     */
    void setUseCase(String name);

    /**
     * LEVEL 1
     *
     * @return all use cases which are supported by the connected module and valid for the
     * current selected CallbackData information (e.g. Raw, Depth, ...)
     * @throws CameraException on royal error
     */
    List<String> getUseCases();

    /**
     * LEVEL 1
     *
     * @return the streams associated with the current use case.
     * @throws CameraException on royal error
     */
    List<Integer> getStreams();

    /**
     * LEVEL 1
     * Retrieves the number of streams for a specified use case.
     *
     * @param useCaseName the name of the use case
     * @throws CameraException on royal error
     */
    long getNumberOfStreams(String useCaseName);

    /**
     * LEVEL 1
     * Gets the current use case as string
     *
     * @return the current use case identified as string
     * @throws CameraException on royal error
     */
    String getCurrentUseCase();

    /**
     * LEVEL 1
     * Change the exposure time for the supported operated operation modes.
     * <p>
     * For mixed-mode use cases a valid streamId must be passed.
     * For use cases having only one stream the default value of 0 (which is otherwise not a valid
     * stream id) can be used to refer to that stream. This is for backward compatibility.
     * <p>
     * If MANUAL exposure mode of operation is chosen, the user is able to determine set
     * exposure time manually within the boundaries of the exposure limits of the specific
     * operation mode.
     * <p>
     * WARNING : If this function is used on Level 3 it will ignore the limits given by the use case.
     *
     * @param exposureTime exposure time in microseconds
     * @param streamId     which stream to change exposure for
     * @throws CameraException with status EXPOSURE_MODE_INVALID, if non-compliance with the selected exposure mode.
     * @throws CameraException with status LOGIC_ERROR, if the camera is used in the playback configuration.
     */
    void setExposureTime(long exposureTime, int streamId);

    /**
     * delegates setExposureTime using 0 as streamId
     *
     * @param exposureTime exposure time in microseconds
     * @throws CameraException on royal error
     */
    default void setExposureTime(long exposureTime)
    {
        setExposureTime(exposureTime, 0);
    }

    /**
     * LEVEL 1
     * Change the exposure mode for the supported operated operation modes.
     * <p>
     * For mixed-mode use cases a valid streamId must be passed.
     * For use cases having only one stream the default value of 0 (which is otherwise not a valid
     * stream id) can be used to refer to that stream. This is for backward compatibility.
     * <p>
     * If MANUAL exposure mode of operation is chosen, the user is able to determine set
     * exposure time manually within the boundaries of the exposure limits of the specific
     * operation mode.
     * <p>
     * In AUTOMATIC mode the optimum exposure settings are determined the system itself.
     * <p>
     * The default value is MANUAL.
     *
     * @param exposureMode mode of operation to determine the exposure time
     * @param streamId     which stream to change exposure mode for
     * @throws CameraException on royal error
     */
    void setExposureMode(ExposureMode exposureMode, int streamId);

    /**
     * delegates setExposureMode using 0 as streamId
     *
     * @param exposureMode mode of operation to determine the exposure time
     * @throws CameraException on royal error
     */
    default void setExposureMode(ExposureMode exposureMode)
    {
        setExposureMode(exposureMode, 0);
    }

    /**
     * LEVEL 1
     * Retrieves the current mode of operation for acquisition of the exposure time.
     * <p>
     * For mixed-mode usecases a valid streamId must be passed.
     * For usecases having only one stream the default value of 0 (which is otherwise not a valid
     * stream id) can be used to refer to that stream. This is for backward compatibility.
     *
     * @param streamId stream for which the exposure mode should be returned
     * @return the current exposure mode
     * @throws CameraException on royal error
     */
    ExposureMode getExposureMode(int streamId);

    /**
     * delegates getExposureMode using 0 as streamId
     *
     * @return the current exposure mode
     * @throws CameraException on royal error
     */
    default ExposureMode getExposureMode()
    {
        return getExposureMode(0);
    }

    /**
     * LEVEL 1
     * Retrieves the minimum and maximum allowed exposure limits of the specified operation
     * mode.  Can be used to retrieve the allowed operational range for a manual definition of
     * the exposure time.
     * <p>
     * For mixed-mode usecases a valid streamId must be passed.
     * For usecases having only one stream the default value of 0 (which is otherwise not a valid
     * stream id) can be used to refer to that stream. This is for backward compatibility.
     *
     * @param streamId stream for which the exposure limits should be returned
     * @return the limits
     * @throws CameraException on royal error
     */
    Pair<Long, Long> getExposureLimits(int streamId);

    /**
     * delegates getExposureMode using 0 as streamId
     *
     * @return the limits
     * @throws CameraException on royal error
     */
    default Pair<Long, Long> getExposureLimits()
    {
        return getExposureLimits(0);
    }

    /**
     * LEVEL 1
     * Once registering the data listener, 3D point cloud data is sent via the callback
     * function.
     *
     * @param listener interface which needs to implement the callback method
     * @throws CameraException on royal error
     */
    void registerDataListener(DepthDataListener listener);

    /**
     * LEVEL 1
     * Unregisters the data depth listener
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     *
     * @throws CameraException on royal error
     */
    void unregisterDataListener();

    /**
     * LEVEL 1
     * Once registering the data listener, Android depth image data is sent via the
     * callback function.
     * <p>
     * Consider using registerDataListener and an IDepthDataListener instead of this listener.
     * This callback provides only an array of depth and confidence values.  The mapping of
     * pixels to the scene is similar to the pixels of a two-dimensional camera, and it is
     * unlikely to be a rectilinear projection (although this depends on the exact camera).
     *
     * @param listener interface which needs to implement the callback method
     * @throws CameraException on royal error
     */
    void registerDepthImageListener(DepthImageListener listener);

    /**
     * LEVEL 1
     * Unregisters the depth image listener
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     *
     * @throws CameraException on royal error
     */
    void unregisterDepthImageListener();

    /**
     * LEVEL 1
     * Once registering the data listener, Android point cloud data is sent via the
     * callback function.
     *
     * @param listener interface which needs to implement the callback method
     * @throws CameraException on royal error
     */
    void registerSparsePointCloudListener(SparsePointCloudListener listener);

    /**
     * LEVEL 1
     * Unregisters the sparse point cloud listener
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     *
     * @throws CameraException on royal error
     */
    void unregisterSparsePointCloudListener();

    /**
     * LEVEL 1
     * Once registering the data listener, IR image data is sent via the callback function.
     *
     * @param listener interface which needs to implement the callback method
     * @throws CameraException on royal error
     */
    void registerIRImageListener(IRImageListener listener);

    /**
     * LEVEL 1
     * Unregisters the IR image listener
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     *
     * @throws CameraException on royal error
     */
    void unregisterIRImageListener();

    /**
     * LEVEL 1
     * Register listener for event notifications.
     * The callback will be invoked asynchronously.
     * Events include things like illumination unit overtemperature.
     *
     * @param listener interface which needs to implement the callback method
     * @throws CameraException on royal error
     */
    void registerEventListener(CameraEventListener listener);

    /**
     * LEVEL 1
     * Unregisters listener for event notifications.
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     *
     * @throws CameraException on royal error
     */
    void unregisterEventListener();

    /**
     * LEVEL 1
     * Starts the video capture mode (free-running), based on the specified operation mode.
     * A listener needs to be registered in order to retrieve the data stream. Either raw data
     * or processed data can be consumed. If no data listener is registered an error will be
     * returned and capturing is not started.
     *
     * @throws CameraException when the camera can not start capturing
     */
    void startCapture();

    /**
     * LEVEL 1
     * Stops the video capturing mode.
     * All buffers should be released again by the data listener.
     *
     * @throws CameraException when the camera can not stop capturing
     */
    void stopCapture();

    /**
     * LEVEL 1
     * Returns the maximal width supported by the camera device.
     *
     * @throws CameraException on royal error
     */
    int getMaxSensorWidth();

    /**
     * LEVEL 1
     * Returns the maximal height supported by the camera device.
     *
     * @throws CameraException on royal error
     */
    int getMaxSensorHeight();

    /**
     * LEVEL 1
     * Gets the intrinsics of the camera module which are stored in the calibration file
     *
     * @return the lens parameter. LensParameters is storing all the relevant information (c,f,p,k)
     * @throws CameraException on royal error
     */
    LensParameters getLensParameters();

    /**
     * LEVEL 1
     * Returns the information if a connection to the camera could be established
     *
     * @return if properly set up
     * @throws CameraException on royal error
     */
    boolean isConnected();

    /**
     * LEVEL 1
     * Returns the information if the camera module is calibrated. Older camera modules
     * can still be operated with royale, but calibration data may be incomplete.
     *
     * @return if the module contains proper calibration data
     * @throws CameraException on royal error
     */
    boolean isCalibrated();

    /**
     * LEVEL 1
     * Returns the information if the camera is currently in capture mode
     *
     * @return if camera is in capture mode
     * @throws CameraException on royal error
     */
    boolean isCapturing();

    /**
     * LEVEL 1
     *
     * @return the current camera device access level
     * @throws CameraException on royal error
     */
    CameraAccessLevel getAccessLevel();

    /**
     * LEVEL 1
     * Start recording the raw data stream into a file.
     * The recording will capture the raw data coming from the imager.
     * If frameSkip and msSkip are both zero every frame will be recorded.
     * If both are non-zero the behavior is implementation-defined.
     *
     * @param fileName       full path of target filename (proposed suffix is .rrf)
     * @param numberOfFrames indicate the maximal number of frames which should be captured
     *                       (stop will be called automatically). If zero (default) is set,
     *                       recording will happen till stopRecording is called.
     * @param frameSkip      indicate how many frames should be skipped after every recorded frame.
     *                       If zero (default) is set and msSkip is zero, every frame will be
     *                       recorded.
     * @param msSkip         indicate how many milliseconds should be skipped after every recorded
     *                       frame. If zero (default) is set and frameSkip is zero, every frame will
     *                       be recorded.
     * @throws CameraException on royal error
     */
    void startRecording(String fileName, long numberOfFrames, long frameSkip, long msSkip);

    /**
     * delegates startRecording using 0 as msSkip
     *
     * @param fileName       full path of target filename (proposed suffix is .rrf)
     * @param numberOfFrames indicate the maximal number of frames which should be captured
     *                       (stop will be called automatically). If zero (default) is set,
     *                       recording will happen till stopRecording is called.
     * @param frameSkip      indicate how many frames should be skipped after every recorded frame.
     *                       If zero (default) is set and msSkip is zero, every frame will be
     *                       recorded.
     * @throws CameraException on royal error
     */
    default void startRecording(String fileName, long numberOfFrames, long frameSkip)
    {
        startRecording(fileName, numberOfFrames, frameSkip, 0);
    }

    /**
     * delegates startRecording using 0 as msSkip and 0 as frameSkip
     *
     * @param fileName       full path of target filename (proposed suffix is .rrf)
     * @param numberOfFrames indicate the maximal number of frames which should be captured
     *                       (stop will be called automatically). If zero (default) is set,
     *                       recording will happen till stopRecording is called.
     * @throws CameraException on royal error
     */
    default void startRecording(String fileName, long numberOfFrames)
    {
        startRecording(fileName, numberOfFrames, 0, 0);
    }

    /**
     * delegates startRecording using 0 as msSkip and 0 as frameSkip and 0 as numberOfFrames
     *
     * @param fileName full path of target filename (proposed suffix is .rrf)
     * @throws CameraException on royal error
     */
    default void startRecording(String fileName)
    {
        startRecording(fileName, 0, 0, 0);
    }

    /**
     * LEVEL 1
     * Stop recording the raw data stream into a file. After the recording is stopped
     * the file is available on the file system.
     *
     * @throws CameraException on royal error
     */
    void stopRecording();

    /**
     * LEVEL 1
     * Once registering a record listener, the listener gets notified once recording
     * has stopped after specified frames.
     *
     * @param listener interface which needs to implement the callback method
     *                 throws CameraException on royal error
     */
    void registerRecordListener(RecordStopListener listener);

    /**
     * LEVEL 1
     * Unregisters the record listener.
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     *
     * @throws CameraException on royal error
     */
    void unregisterRecordListener();

    /**
     * LEVEL 1
     * Once registering the exposure listener, new exposure values calculated by the
     * processing are sent to the listener.
     *
     * @param listener interface which needs to implement the callback method
     */
    void registerExposureListener(ExposureListener listener);

    /**
     * LEVEL 1
     * Unregisters the exposure listener
     * <p>
     * It's not necessary to unregister this listener (or any other listener) before deleting
     * the ICameraDevice.
     */
    void unregisterExposureListener();

    /**
     * LEVEL 1
     * Set the frame rate to a value. Upper bound is given by the use case.
     * E.g. Usecase with 5 FPS, a maximum frame rate of 5 and a minimum of 1 can be set.
     * Setting a frame rate of 0 is not allowed.
     * This function is not supported for mixed-mode.
     *
     * @param frameRate the frameRate is specific for the current use case.
     * @throws CameraException on royal error
     */
    void setFrameRate(int frameRate);

    /**
     * LEVEL 1
     * Get the current frame rate which is set for the current use case.
     * This function is not supported for mixed-mode.
     *
     * @return the frame rate
     * @throws CameraException on royal error
     */
    int getFrameRate();

    /**
     * LEVEL 1
     * Get the maximal frame rate which can be set for the current use case.
     * This function is not supported for mixed-mode.
     *
     * @return the maximum frame rate
     * @throws CameraException on royal error
     */
    int getMaxFrameRate();

    /**
     * LEVEL 1
     * Enable or disable the external triggering.
     * Some camera modules support an external trigger, they can capture images synchronized with another device.
     * If the hardware you are using supports it, calling setExternalTrigger(true) will make the camera capture images in this way.
     * The call to setExternalTrigger has to be done before initializing the device.
     * <p>
     * The external signal must not exceed the maximum FPS of the chosen UseCase, but lower frame rates are supported.
     * If no external signal is received, the imager will not start delivering images.
     * <p>
     * For information if your camera module supports external triggering and how to use it please refer to
     * the Getting Started Guide of your camera. If the module doesn't support triggering calling this function
     * will return a LOGIC_ERROR.
     * <p>
     * Royale currently expects a trigger pulse, not a ant trigger signal. Using a ant
     * trigger signal might lead to a wrong framerate!
     *
     * @param useExternalTrigger the external trigger to set
     * @throws CameraException on royal error
     */
    void setExternalTrigger(boolean useExternalTrigger);
}
