/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using HND_TYPE = System.UInt64;

namespace RoyaleDotNet
{
    /// <summary>
    /// The CameraManager is responsible for detecting and creating a `CameraDevice`
    /// from a connected camera. Depending on the provided activation code access `Level 2` or
    /// `Level 3` can be created. Due to eye safety reasons, `Level 3` is for internal purposes only.
    /// Once a known time-of-flight device is detected, the according communication (e.g. via USB)
    /// is established and the camera device is ready.
    /// </summary>
    public sealed class CameraManager
    {
        #region Interop DLL Stub
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_string_array2 (out IntPtr string_array, UInt32 nr_strings);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_camera_manager_create_with_code ([MarshalAs (UnmanagedType.LPStr)] string activationCode);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_camera_manager_destroy (HND_TYPE cameraManagerHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern IntPtr royale_camera_manager_get_connected_cameras_android (HND_TYPE cameraManagerHnd, ref UInt32 numCameras, UInt32 androidUsbFD,
            UInt32 androidUsbVid, UInt32 androidUsbPid);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_camera_manager_create_camera (HND_TYPE cameraManagerHnd, [MarshalAs (UnmanagedType.LPStr)] string cameraID);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_camera_manager_create_camera_with_trigger (HND_TYPE cameraManagerHnd, [MarshalAs (UnmanagedType.LPStr)] string cameraID, UInt16 triggerMode);
        #endregion

        private HND_TYPE m_cameraManagerHnd;

        /// <summary>
        /// Constructor of the CameraManager. An empty activationCode only allows to get an ICameraDevice.
        /// A valid activation code also allows to gain higher level access rights.
        /// </summary>
        public CameraManager (string activationCode = "")
        {
            m_cameraManagerHnd = royale_camera_manager_create_with_code (activationCode);
            if (m_cameraManagerHnd == 0)
            {
                throw new Exception ("Could not instantiate CameraManager");
            }
        }

        /// <summary>
        /// Deletes the CameraManager instance and releases all allocated resources.
        /// </summary>
        ~CameraManager()
        {
            royale_camera_manager_destroy (m_cameraManagerHnd);
        }

        /// <summary>
        /// Returns the list of connected camera modules identified by a unique ID (serial number).
        /// This call tries to connect to each plugged-in camera and queries for its unique serial number.
        /// Once the scope of CameraManager ends, all (other) unused CameraDevices will
        /// be closed automatically. The CreateCamera() keeps the CameraDevice beyond the scope
        /// of the CameraManager since the ownership is given to the caller.
        /// </summary>
        /// <param name="androidUsbDeviceFD">ONLY REQUIRED ON ANDROID! File descriptor of the USB camera.</param>
        /// <param name="androidUsbDeviceVid">ONLY REQUIRED ON ANDROID! Vendor ID of the USB camera.</param>
        /// <param name="androidUsbDevicePid">ONLY REQUIRED ON ANDROID! Product ID of the USB camera.</param>
        /// <returns>List of connected camera modules</returns>
        public List<string> GetConnectedCameraList (UInt32 androidUsbDeviceFD = UInt32.MaxValue,
            UInt32 androidUsbDeviceVid = UInt32.MaxValue,
            UInt32 androidUsbDevicePid = UInt32.MaxValue)
        {
            List<string> connectedCameras = new List<string>();

            UInt32 numCameras = 0;
            IntPtr connectedCamerasNative = royale_camera_manager_get_connected_cameras_android (m_cameraManagerHnd, ref numCameras, androidUsbDeviceFD,
                androidUsbDeviceVid, androidUsbDevicePid);

            if (numCameras > 0)
            {
                IntPtr[] stringPointers = new IntPtr[numCameras];
                Marshal.Copy (connectedCamerasNative, stringPointers, 0, (int) numCameras);
                foreach (IntPtr stringPtr in stringPointers)
                {
                    connectedCameras.Add (Marshal.PtrToStringAnsi (stringPtr));
                }
                royale_free_string_array2 (out connectedCamerasNative, numCameras);
            }

            return connectedCameras;
        }

        /// <summary>
        /// Creates the camera object ICameraDevice identified by its ID. If the ID
        /// is not correct, a nullptr will be returned. The ownership is transfered to the caller, which
        /// means that the ICameraDevice is still valid once the CameraManager is out of scope.
        /// </summary>
        /// <param name="cameraID">Unique ID of the camera which was returned from GetConnectedCameraList()</param>
        /// <returns>CameraDevice instance if ID was found, null otherwise</returns>
        public CameraDevice CreateCamera (string cameraID)
        {
            HND_TYPE cameraDeviceHnd = royale_camera_manager_create_camera (m_cameraManagerHnd, cameraID);
            if (cameraDeviceHnd == 0)
            {
                return null;
            }
            return new CameraDevice (cameraDeviceHnd);
        }

        /// <summary>
        /// Creates the camera object ICameraDevice identified by its ID and sets the specified trigger mode
        /// </summary>
        /// <param name="cameraID">Unique ID of the camera which was returned from GetConnectedCameraList()</param>
        /// <param name="triggerMode">Trigger mode that should be used</param>
        /// <returns>CameraDevice instance if ID was found, null otherwise</returns>
        public CameraDevice CreateCamera(string cameraID, TriggerMode triggerMode)
        {
            HND_TYPE cameraDeviceHnd = royale_camera_manager_create_camera_with_trigger(m_cameraManagerHnd, cameraID, (UInt16) triggerMode);
            if (cameraDeviceHnd == 0)
            {
                return null;
            }
            return new CameraDevice(cameraDeviceHnd);
        }
    }
}
