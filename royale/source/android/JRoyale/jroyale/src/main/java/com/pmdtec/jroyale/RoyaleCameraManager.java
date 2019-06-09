/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale;

import android.app.*;
import android.content.*;
import android.hardware.usb.*;

import java.util.*;

class RoyaleCameraManager implements CameraManager
{
    /**
     * an identifier for usb-permission requests
     * contains the canonical name of the class an the field itself
     */
    private static final String USB_PERMISSION = "com.pmdtec.jroyale.CameraManager#USB_PERMISSION";

    static
    {
        /*
         * we have to ensure that the following libraries are loaded before we can use this class
         * else the native methods in this class are not wired
         */
        System.loadLibrary("jroyale");
    }

    /**
     * notice that the mHandle fields are only used by JNI to store object specific cpp-pointer
     */
    @SuppressWarnings("unused")
    private long mHandle;

    /**
     * Default Constructor
     * <p>
     * when the camera-manager is created the associated c-object
     * in this case a royale::CameraManager will also be created
     * the resulting c-pointer to this object will then be stored in the mNativeHandle field
     */
    RoyaleCameraManager(String activationCode)
    {
        initializeNative(activationCode);
    }

    @Override
    protected void finalize() throws Throwable
    {
        finalizeNative();
        super.finalize();
    }

    /**
     */
    private native void initializeNative(String activationCode);

    /**
     */
    private native void finalizeNative();

    @Override
    public void createCamera(Context context, CameraCreationCallback callback)
    {
        UsbManager usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        if (null == usbManager)
        {
            callback.onError(CameraCreationError.USB_SERVICE_ERROR);
            return;
        }

        BroadcastReceiver receiver = new MyBroadcastReceiver(context, usbManager, callback);
        context.registerReceiver(receiver, new IntentFilter(USB_PERMISSION));

        Map<String, UsbDevice> deviceList = usbManager.getDeviceList();
        for (UsbDevice usbDevice : deviceList.values())
        {
            int vendorId = usbDevice.getVendorId();

            // magic numbers are defined by the camera hardware
            //
            if ((0x1C28 == vendorId) || (0x058B == vendorId) || (0x1f46 == vendorId))
            {
                if (usbManager.hasPermission(usbDevice))
                {
                    CameraDevice cameraDevice;
                    try
                    {
                        cameraDevice = createCamera(usbManager, usbDevice);
                    }
                    catch (CameraException ignore)
                    {
                        callback.onError(CameraCreationError.RUNTIME_ERROR);
                        return;
                    }

                    // we do not want to catch an error that is done externally ...
                    // therefor the call is excluded
                    //
                    callback.onOpen(cameraDevice);
                }
                else
                {
                    Intent intent = new Intent(USB_PERMISSION);
                    PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, 0);
                    usbManager.requestPermission(usbDevice, pendingIntent);
                }
            }
        }
    }

    @Override
    public native CameraAccessLevel getAccessLevel(String activationCode);

    /**
     * @param fd  the file descriptor
     * @param vid the version id
     * @param pid the product id
     * @return the created camera device
     */
    @Override
    public native CameraDevice createCamera(long fd, long vid, long pid);

    /**
     * @param path the path to a rrf-file
     * @return the created {@link RoyaleCameraDevice}
     */
    @Override
    public native CameraDevice createCamera(String path);

    /**
     * a helper class to receive the usb permission request'S answer
     *
     * @see RoyaleCameraManager#createCamera(android.content.Context,
     * CameraCreationCallback)
     */
    private class MyBroadcastReceiver extends BroadcastReceiver
    {
        private Context mContext;
        private UsbManager mUsbManager;
        private CameraCreationCallback mCameraCallback;

        MyBroadcastReceiver(Context context, UsbManager usbManager,
                            CameraCreationCallback cameraCallback)
        {
            mContext = context;
            mUsbManager = usbManager;
            mCameraCallback = cameraCallback;
        }

        /**
         * delegates to the callback
         */
        @Override
        public void onReceive(Context context, Intent intent)
        {
            mContext.unregisterReceiver(this);

            UsbDevice usbDevice = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (null == usbDevice)
            {
                mCameraCallback.onError(CameraCreationError.INTENT_ERROR);
            }
            else if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false))
            {
                CameraDevice cameraDevice;
                try
                {
                    cameraDevice = createCamera(mUsbManager, usbDevice);
                }
                catch (CameraException ignore)
                {
                    mCameraCallback.onError(CameraCreationError.RUNTIME_ERROR);
                    return;
                }

                // we do not want to catch an error that is done externally ...
                // therefor the call is excluded
                //
                mCameraCallback.onOpen(cameraDevice);
            }
            else
            {
                mCameraCallback.onError(CameraCreationError.USB_PERMISSION_DENIED);
            }
        }
    }
}
