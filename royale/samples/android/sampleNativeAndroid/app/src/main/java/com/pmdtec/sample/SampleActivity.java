package com.pmdtec.sample;

import android.app.*;
import android.content.*;
import android.graphics.*;
import android.hardware.usb.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.widget.*;

import java.util.*;

public class SampleActivity extends Activity
{
    private static final String TAG = "RoyaleAndroidSampleV3";
    private static final String ACTION_USB_PERMISSION = "ACTION_ROYALE_USB_PERMISSION";

    private UsbManager mUSBManager;
    private UsbDeviceConnection mUSBConnection;

    private Bitmap mBitmap;
    private ImageView mAmplitudeView;

    private boolean mOpened;

    private int mScaleFactor;
    private int[] mResolution;

    /**
     * broadcast receiver for user usb permission dialog
     */
    private BroadcastReceiver mUsbReceiver = new BroadcastReceiver()
    {
        @Override
        public void onReceive(Context context, Intent intent)
        {
            Log.i(TAG, "SampleActivity.onReceive context = [" + context + "], intent = [" + intent + "]");

            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action))
            {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false))
                {
                    if (device != null)
                    {
                        NativeCamera.registerAmplitudeListener(SampleActivity.this::onAmplitudes);
                        performUsbPermissionCallback(device);
                        createBitmap();
                    }
                }
                else
                {
                    System.out.println("permission denied for device" + device);
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.i(TAG, "SampleActivity.onCreate savedInstanceState = [" + savedInstanceState + "]");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sample);

        Log.d(TAG, "onCreate()");

        Button btnStart = findViewById(R.id.buttonStart);
        mAmplitudeView = findViewById(R.id.imageViewAmplitude);
        btnStart.setOnClickListener(view -> openCamera());
    }

    /**
     * Will be invoked on a new frame captured by the camera.
     *
     * @see com.pmdtec.sample.NativeCamera.AmplitudeListener
     */
    public void onAmplitudes(int[] amplitudes)
    {
        if (!mOpened)
        {
            Log.d(TAG, "Device in Java not initialized");
            return;
        }

        mBitmap.setPixels(amplitudes, 0, mResolution[0], 0, 0, mResolution[0], mResolution[1]);

        runOnUiThread(() -> mAmplitudeView.setImageBitmap(Bitmap.createScaledBitmap(mBitmap,
                mResolution[0] * mScaleFactor,
                mResolution[1] * mScaleFactor, false)));
    }

    public void openCamera()
    {
        Log.i(TAG, "SampleActivity.openCamera");

        //check permission and request if not granted yet
        mUSBManager = (UsbManager) getSystemService(Context.USB_SERVICE);

        if (mUSBManager != null)
        {
            Log.d(TAG, "Manager valid");
        }

        HashMap<String, UsbDevice> deviceList = mUSBManager.getDeviceList();

        Log.d(TAG, "USB Devices : " + deviceList.size());

        Iterator<UsbDevice> iterator = deviceList.values().iterator();
        UsbDevice device;
        boolean found = false;
        while (iterator.hasNext())
        {
            device = iterator.next();
            if (device.getVendorId() == 0x1C28 ||
                    device.getVendorId() == 0x058B ||
                    device.getVendorId() == 0x1f46)
            {
                Log.d(TAG, "royale device found");
                found = true;
                if (!mUSBManager.hasPermission(device))
                {
                    Intent intent = new Intent(ACTION_USB_PERMISSION);
                    intent.setAction(ACTION_USB_PERMISSION);
                    PendingIntent mUsbPi = PendingIntent.getBroadcast(this, 0, intent, 0);
                    mUSBManager.requestPermission(device, mUsbPi);
                }
                else
                {
                    NativeCamera.registerAmplitudeListener(this::onAmplitudes);
                    performUsbPermissionCallback(device);
                    createBitmap();
                }
                break;
            }
        }
        if (!found)
        {
            Log.e(TAG, "No royale device found!!!");
        }
    }

    private void performUsbPermissionCallback(UsbDevice device)
    {
        Log.i(TAG, "SampleActivity.performUsbPermissionCallback device = [" + device + "]");

        mUSBConnection = mUSBManager.openDevice(device);
        Log.i(TAG, "permission granted for: " + device.getDeviceName() + ", fileDesc: " + mUSBConnection.getFileDescriptor());

        int fd = mUSBConnection.getFileDescriptor();

        mResolution = NativeCamera.openCameraNative(fd, device.getVendorId(), device.getProductId());

        if (mResolution[0] > 0)
        {
            mOpened = true;
        }
    }

    private void createBitmap()
    {
        // calculate scale factor, which scales the bitmap relative to the display mResolution
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        double displayWidth = size.x * 0.9;
        mScaleFactor = (int) displayWidth / mResolution[0];

        if (mBitmap == null)
        {
            mBitmap = Bitmap.createBitmap(mResolution[0], mResolution[1], Bitmap.Config.ARGB_8888);
        }
    }

    @Override
    protected void onPause()
    {
        Log.i(TAG, "SampleActivity.onPause");
        super.onPause();

        if (mOpened)
        {
            NativeCamera.closeCameraNative();
            mOpened = false;
        }

        unregisterReceiver(mUsbReceiver);
    }

    @Override
    protected void onResume()
    {
        Log.i(TAG, "SampleActivity.onResume");
        super.onResume();

        registerReceiver(mUsbReceiver, new IntentFilter(ACTION_USB_PERMISSION));
    }

    @Override
    protected void onDestroy()
    {
        Log.i(TAG, "SampleActivity.onDestroy");
        super.onDestroy();

        Log.d(TAG, "onDestroy()");
        unregisterReceiver(mUsbReceiver);

        if (mUSBConnection != null)
        {
            mUSBConnection.close();
        }
    }
}
