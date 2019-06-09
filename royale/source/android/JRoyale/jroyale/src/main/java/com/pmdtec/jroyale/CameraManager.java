/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale;

import android.content.*;
import android.hardware.usb.*;

/**
 * The CameraManager is responsible for detecting and creating instances of `ICameraDevices`
 * one for each connected (supported) camera device. Depending on the provided activation code
 * access `Level 2` or `Level 3` can be created. Due to eye safety reasons, `Level 3` is for
 * internal purposes only. Once a known time-of-flight device is detected, the according
 * communication (e.g. via USB) is established and the camera device is ready.
 */
public interface CameraManager
{
    /**
     * Constructor of the CameraManager.
     *
     * @return cameraManager that allows to get an ICameraDevice
     */
    static CameraManager createRoyaleCameraManager()
    {
        return new RoyaleCameraManager("");
    }

    /**
     * Constructor of the CameraManager. An empty activationCode only allows to get an ICameraDevice.
     * A valid activation code also allows to gain `Level 2` or `Level 3 ` access rights.
     *
     * @param activationCode the activation code
     * @return cameraManager that with or without an Access Level
     */
    static CameraManager createRoyaleCameraManager(String activationCode)
    {
        return new RoyaleCameraManager(activationCode);
    }

    /**
     * Retrieves the access level to the given activation code.
     *
     * @param activationCode the activation code
     * @return the camera access level
     */
    CameraAccessLevel getAccessLevel(String activationCode);

    /**
     * Creates a master or slave camera object CameraDevice identified by its UsbDevice object and
     * the Manager to create
     * <p>
     * If the camera is opened as a slave it will not receive a start signal from Royale, but will
     * wait for the external trigger signal. Please have a look at the master/slave example which
     * shows how to deal with multiple cameras.
     *
     * @param usbManager the manager to create the camera instance
     * @param usbDevice  the chosen device to connect
     * @return the created {@link RoyaleCameraDevice} instance
     * @throws CameraException when royale can not find an royale device using the given
     *                         parameters
     */
    default CameraDevice createCamera(UsbManager usbManager, UsbDevice usbDevice)
    {
        UsbDeviceConnection usbConnection = usbManager.openDevice(usbDevice);

        long fd = usbConnection.getFileDescriptor();
        long vid = usbDevice.getVendorId();
        long pid = usbDevice.getProductId();

        return createCamera(fd, vid, pid);
    }

    /**
     * @param fd  the file descriptor
     * @param vid the version id
     * @param pid the product id
     * @return the created camera device
     */
    CameraDevice createCamera(long fd, long vid, long pid);

    /**
     * Creates a master or slave camera object CameraDevice identified by the rrf-file path
     * <p>
     * If the camera is opened as a slave it will not receive a start signal from Royale, but will
     * wait for the external trigger signal. Please have a look at the master/slave example which
     * shows how to deal with multiple cameras.
     *
     * @param path a path to a valid rrf-file
     * @return the created {@link RoyaleCameraDevice} instance
     * @throws CameraException when royale can not find a the rrf-file
     */
    CameraDevice createCamera(String path);

    /**
     * find an royale ubs device and collects the permission to access its resource
     * <p>
     * afterwards delegates the
     * {@link RoyaleCameraManager#createCamera(android.hardware.usb.UsbManager,
     * android.hardware.usb.UsbDevice)} method to create a {@link RoyaleCameraDevice}
     * instance
     *
     * @param context  the context to receive the usb system service from
     * @param callback the callback to pass the created {@link RoyaleCameraDevice} into
     */
    void createCamera(Context context, CameraCreationCallback callback);
}
