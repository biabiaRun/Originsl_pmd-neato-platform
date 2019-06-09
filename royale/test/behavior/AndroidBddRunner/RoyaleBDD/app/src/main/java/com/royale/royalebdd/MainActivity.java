package com.royale.royalebdd;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.view.Display;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Iterator;

public class MainActivity extends Activity {

    private PendingIntent mUsbPi;
    private UsbManager manager;

    private static final String LOG_TAG = "ROYALE";
    private static final String ACTION_USB_PERMISSION = "ACTION_ROYALE_USB_PERMISSION";

    private static int fd;

    private String[] libraryFileNames = {
            "libusb_android.so",
            "libroyale.so",
            "libtest_royale_bdd.so"
    };

    public native int runBDD(int fd);

    //broadcast receiver for user usb permission dialog
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                    if (device != null) {
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
        setContentView(R.layout.activity_main);
        Log.d(LOG_TAG, "onCreate()");
        fetchLibrariesFromInternalStorage();

        File folder = new File(Environment.getExternalStorageDirectory() + "/bdd");
        boolean success = true;
        if (!folder.exists()) {
            success = folder.mkdir();
        }
        if (success) {
            Log.e("ROYALE_BDD", "OK");
        } else {
            Log.e("ROYALE_BDD", "failed");
        }

        openCamera();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(LOG_TAG, "onPause()");
        unregisterReceiver(mUsbReceiver);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(LOG_TAG, "onResume()");
        registerReceiver(mUsbReceiver, new IntentFilter(ACTION_USB_PERMISSION));
    }

    @Override
    protected void onDestroy() {
        Log.d(LOG_TAG, "onDestroy()");
        unregisterReceiver(mUsbReceiver);
        super.onDestroy();
    }

    private void performUsbPermissionCallback(UsbDevice device) {
        UsbDeviceConnection conn = manager.openDevice(device);
        Log.i(LOG_TAG, "Permission granted for: " + device.getDeviceName() + ", fileDesc: " + conn.getFileDescriptor());

        fd = conn.getFileDescriptor();

        Log.e("ROYALE_BDD", "runBDD: " + runBDD(fd));
        Log.e("ROYALE_BDD", "done");

        conn.close();
    }

    private boolean fetchLibrariesFromInternalStorage(){
        String sourcePath = Environment.getExternalStorageDirectory().getPath() + "/bdd/";
        String outputPath = getFilesDir().getPath() + "/";
        try {
            for(String lib : libraryFileNames) {
                String targetFileName = outputPath + lib;
                FileInputStream fis = new FileInputStream(sourcePath + lib);
                File nf = new File(targetFileName);
                if(nf.exists()){
                    nf.delete();
                }
                FileOutputStream fos = new FileOutputStream(nf);
                byte[] buf = new byte[4096];
                int n;
                while ((n = fis.read(buf)) > 0)
                    fos.write(buf, 0, n);
                fis.close();
                fos.close();
                System.load(targetFileName);
            }
        } catch (Exception ex){
            Log.e(LOG_TAG, "ERROR fetching libraries: " + ex.getMessage());
        }
        return true;
    }

    public void openCamera() {
        Log.d(LOG_TAG, "openCamera");

        //check permission and request if not granted yet
        manager = (UsbManager) getSystemService(Context.USB_SERVICE);

        if (manager != null) {
            Log.d(LOG_TAG, "Manager valid");
        }

        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();

        Log.d(LOG_TAG, "USB Devices : " + deviceList.size());

        Iterator<UsbDevice> iterator = deviceList.values().iterator();
        UsbDevice device;
        boolean found = false;
        while (iterator.hasNext()) {
            device = iterator.next();
            if (device.getVendorId() == 0x1C28) {
                Log.d(LOG_TAG, "royale device found");
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
            Log.e(LOG_TAG, "No royale device found!!!");
        }
    }

}
