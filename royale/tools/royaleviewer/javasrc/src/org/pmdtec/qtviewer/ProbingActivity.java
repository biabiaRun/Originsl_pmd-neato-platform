/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

package org.pmdtec.qtviewer;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;

import org.qtproject.qt5.android.bindings.QtActivity;

import java.util.HashMap;
import java.util.Iterator;

import android.view.View;
import android.app.ActionBar;

public class ProbingActivity extends QtActivity {

    static
    { //do not reorder this, native libraries have to be loaded in reverse dependency order
        System.loadLibrary("usb_android");
        System.loadLibrary("royale");
    }

    private PendingIntent mUsbPi;
    private UsbManager manager;
    private UsbDeviceConnection usbConnection;
    
    private static final String LOG_TAG = "ROYALE";
    private static final String ACTION_USB_PERMISSION = "ACTION_ROYALE_USB_PERMISSION";
    private static final String EXTRA_NAME_QTVIEWER_HANDLE = "EXTRA_NAME_QTVIEWER_HANDLE";
    public static native void ProbingResult(int fd, int vid, int pid);
    public static native void SetActivationCode(String activationCode);

    //broadcast receiver for user usb permission dialog
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                    if (device != null) {
                        int qtViewerHandle = intent.getIntExtra(EXTRA_NAME_QTVIEWER_HANDLE, 0);
                        performUsbPermissionCallback(device);
                    }
                } else {
                    System.out.println("permission denied for device" + device);
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        View decorView = getWindow().getDecorView();
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        ActionBar actionBar = getActionBar();
        actionBar.hide();
        
        Intent intent = getIntent();
        if (intent.hasExtra("activationCode"))
        {
            String activationCode = intent.getStringExtra("activationCode");
            SetActivationCode(activationCode);
        }
        
        openCamera ();
    }

    @Override
    protected void onPause() {
        super.onPause();
        try {
            unregisterReceiver(mUsbReceiver);
        }
        catch (final Exception exception) {
            // The receiver was not registered.
            // There is nothing to do in that case.
            // Everything is fine.
            Log.d(LOG_TAG, "Exception caught during unregistering of the receiver!!!");
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);

        if (intent.hasExtra("activationCode"))
        {
            String activationCode = intent.getStringExtra("activationCode");
            SetActivationCode(activationCode);
        }
        else
        {
            SetActivationCode (""); // ensure that an L1 launch doesn't get access from a previous launch
        }
    }    
    
    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(mUsbReceiver, new IntentFilter(ACTION_USB_PERMISSION));
    }

    @Override
    protected void onDestroy() {
        try {
            unregisterReceiver(mUsbReceiver);
        }
        catch (final Exception exception) {
            // The receiver was not registered.
            // There is nothing to do in that case.
            // Everything is fine.
            Log.d(LOG_TAG, "Exception caught during unregistering of the receiver!!!");
        }
        
        if(usbConnection != null) {
            usbConnection.close();
        }

        super.onDestroy();
    }

    public void openCamera() {
        //check permission and request if not granted yet
        manager = (UsbManager) getSystemService(Context.USB_SERVICE);

        if (manager != null) {
            Log.d(LOG_TAG, "Manager valid");
        }

        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();

        Log.d(LOG_TAG, "Devices : " + deviceList.size());

        Iterator<UsbDevice> iterator = deviceList.values().iterator();
        UsbDevice device;
        boolean found = false;
        while (iterator.hasNext()) {
            device = iterator.next();
            if (device.getVendorId() == 0x1C28 || 
                device.getVendorId() == 0x058B ||
                device.getVendorId() == 0x1f46) {
                Log.d(LOG_TAG, "Found compatible device(s)");
                found = true;
                if (!manager.hasPermission(device)) {
                    Intent intent = new Intent(ACTION_USB_PERMISSION);
                    intent.setAction(ACTION_USB_PERMISSION);
                    mUsbPi = PendingIntent.getBroadcast(this, 0, intent, 0);
                    manager.requestPermission(device, mUsbPi);
                } else {
                    performUsbPermissionCallback(device);
                }
                break;
            }
        }
        if (!found) {
            Log.d(LOG_TAG, "No compatible devices found!!!");
        }
    }

    private void performUsbPermissionCallback(UsbDevice device) {
        usbConnection = manager.openDevice(device);
        Log.i(LOG_TAG, "permission granted for: " + device.getDeviceName() + ", fileDesc: " + usbConnection.getFileDescriptor());

        int fd = usbConnection.getFileDescriptor();

        ProbingResult(fd, device.getVendorId(), device.getProductId());
    }
}
