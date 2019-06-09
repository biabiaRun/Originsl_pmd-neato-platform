package com.pmdtec.jroyale;

/**
 *
 */
public enum CameraCreationError
{
    /**
     * permission denied for an {@link android.hardware.usb.UsbDevice}
     */
    USB_PERMISSION_DENIED,

    /**
     * the intent does not contain an {@link android.hardware.usb.UsbDevice}
     */
    INTENT_ERROR,

    /**
     * an unknown runtime error occurred camera can not be created
     * <p>
     * in most cases this indicates that the passed parameters to create a camera are bad
     */
    RUNTIME_ERROR,

    /**
     * the usb manager can not get the usb system service
     * <p>
     * the usb system service is needed by the camera
     * manager in order to create a camera
     */
    USB_SERVICE_ERROR
}
