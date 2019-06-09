/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
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
using System.Text;
using HND_TYPE = System.UInt64;

namespace RoyaleDotNet
{
    /// <summary>
    /// This class represents the time-of-flight camera system. Typically,
    /// an instance is created by the `CameraManager` which automatically detects a connected module.
    /// The support access levels can be activated by entering the correct code during creation.
    /// After creation, the CameraDevice is in ready state and can be initialized. Before starting
    /// capturing an operation mode must be set. The provided examples should give a good overview
    /// on how to use this class.
    /// </summary>
    public sealed class CameraDevice
    {
        #region Interop DLL Stub

        #region Utils

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_string (IntPtr dest);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_lens_parameters (ref LensParameters.NativeLensParameters nativeParams);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_vector_stream_id (ref StreamId.NativeVectorStreamId nativeVector);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_string_array2 (ref IntPtr string_array, UInt32 nr_strings);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_pair_string_string_array (out IntPtr pair_array, UInt32 nr_elements);

        [StructLayout (LayoutKind.Sequential)]
        private struct NativePairStringUInt64
        {
            [MarshalAs (UnmanagedType.LPStr)] public string first;
            public UInt64 second;
        }
        #endregion

        #region Level 1
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_camera_device_destroy_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_initialize_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_id_v220 (HND_TYPE cameraDeviceHnd, out IntPtr idPtr);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_camera_name_v220 (HND_TYPE cameraDeviceHnd, out IntPtr idPtr);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_camera_info_v220 (HND_TYPE cameraDeviceHnd, out IntPtr infoPtr, ref UInt32 numData);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_start_capture_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_stop_capture_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_data_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_data_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_depth_image_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_depth_image_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_spc_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_spc_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_record_stop_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_record_stop_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_exposure_listener_stream_v300 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_exposure_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_ir_image_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_ir_image_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_event_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_event_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_max_sensor_width_v220 (HND_TYPE cameraDeviceHnd, out UInt16 maxSensorWidth);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_max_sensor_height_v220 (HND_TYPE cameraDeviceHnd, out UInt16 maxSensorHeight);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_use_cases_v220 (HND_TYPE cameraDeviceHnd, out IntPtr useCasesPtr, out UInt32 numUseCases);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_streams_v300 (HND_TYPE cameraDeviceHnd, ref StreamId.NativeVectorStreamId streams);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_number_of_streams_v330 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.LPStr)] string useCase, out UInt32 numStreams);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_current_use_case_v220 (HND_TYPE cameraDeviceHnd, out IntPtr useCasePtr);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_use_case_v210 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.LPStr)] string useCase);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_is_connected_v220 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.I1)] out bool isConnected);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_is_calibrated_v220 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.I1)] out bool isCalibrated);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_is_capturing_v220 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.I1)] out bool isCapturing);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_access_level_v220 (HND_TYPE cameraDeviceHnd, out CameraAccessLevel accessLevel);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_exposure_time_v300 (HND_TYPE cameraDeviceHnd, UInt16 streamId, UInt32 exposureTime);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_exposure_mode_v300 (HND_TYPE cameraDeviceHnd, UInt16 streamId, out ExposureMode exposureMode);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_exposure_mode_v300 (HND_TYPE cameraDeviceHnd, UInt16 streamId, UInt32 exposureMode);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_frame_rate_v210 (HND_TYPE cameraDeviceHnd, UInt16 frameRate);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_frame_rate_v220 (HND_TYPE cameraDeviceHnd, out UInt16 frameRate);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_max_frame_rate_v220 (HND_TYPE cameraDeviceHnd, out UInt16 maxFrameRate);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_external_trigger_v330 (HND_TYPE cameraDeviceHnd, bool useExternalTrigger);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_exposure_limits_v300 (HND_TYPE cameraDeviceHnd, UInt16 streamId, out UInt32 lowerLimit, out UInt32 upperLimit);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 royale_camera_device_get_lens_parameters_v210 (HND_TYPE cameraDeviceHnd, ref LensParameters.NativeLensParameters nativeLensParams);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_start_recording_v210 (HND_TYPE cameraDeviceHnd,
                [MarshalAs (UnmanagedType.LPStr)] string fileName, UInt32 numberOfFrames, UInt32 framesSkip, UInt32 msSkip);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_stop_recording_v210 (HND_TYPE cameraDeviceHnd);
        #endregion

        #region Level 2
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_exposure_groups_v300 (HND_TYPE cameraDeviceHnd, out IntPtr namesPtr, out UInt32 numExposureGroups);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_exposure_times_v300 (HND_TYPE cameraDeviceHnd, UInt16 streamId, UInt32[] exposureTimes, UInt32 numExposureTimes);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_exposure_for_groups_v300 (HND_TYPE cameraDeviceHnd, UInt32[] exposureTimes, UInt32 numExposureTimes);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_group_exposure_time_v300 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.LPStr)] string exposureGroup, UInt32 exposureTime);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_group_exposure_limits_v300 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.LPStr)] string exposureGroup, out UInt32 lowerLimit, out UInt32 upperLimit);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_managed_set_proc_params (HND_TYPE cameraDeviceHnd, UInt16 streamId,
                UInt32[] processingParameterArray, UInt32 totalLength);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_managed_get_proc_params (HND_TYPE cameraDeviceHnd, UInt16 streamId, IntPtr callback);

        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        private delegate void GetProcessingParameterNativeCallback (IntPtr data, UInt32 totalLength);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_register_extended_data_listener_v210 (HND_TYPE cameraDeviceHnd, IntPtr callback);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_unregister_extended_data_listener_v210 (HND_TYPE cameraDeviceHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_callback_data_v210 (HND_TYPE cameraDeviceHnd, UInt16 cbData);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_calibration_data_file (HND_TYPE cameraDeviceHnd,
                [MarshalAs (UnmanagedType.LPStr)] string calibrationFileName);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_calibration_data_v210 (HND_TYPE cameraDeviceHnd,
                ref IntPtr valuesArray, UInt32 numberOfElements);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_managed_get_calib_data (HND_TYPE handle, IntPtr callback);

        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        private delegate void GetCalibrationDataNativeCallback (IntPtr data, UInt32 totalLength);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 CameraDeviceGetCalibrationData (HND_TYPE cameraDeviceHnd,  IntPtr callback);

        [UnmanagedFunctionPointer (CallingConvention.Cdecl)]
        private delegate void GetGetCalibrationDataNativeCallback (IntPtr data, UInt32 totalLength);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_write_calibration_to_flash_v210 (HND_TYPE cameraDeviceHnd);
        #endregion

        #region Level 3
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_set_duty_cycle_v210 (HND_TYPE cameraDeviceHnd, double dutyCycle, UInt16 index);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 royale_camera_device_write_registers_v210 (HND_TYPE cameraDeviceHnd, IntPtr[] arrayPtr, UInt32 numberOfElements);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_read_registers_v210 (HND_TYPE cameraDeviceHnd, IntPtr[] arrayPtr, UInt32 numberOfElements);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_shift_lens_center_v320 (HND_TYPE cameraDeviceHnd, Int16 tx, Int16 ty);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_get_lens_center_v320 (HND_TYPE cameraDeviceHnd, out UInt16 x, out UInt16 y);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_write_data_to_flash_v31000 (HND_TYPE cameraDeviceHnd,
                ref IntPtr valuesArray, UInt32 numberOfElements);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_write_data_to_flash_file_v31000 (HND_TYPE cameraDeviceHnd,
                [MarshalAs (UnmanagedType.LPStr)] string fileName);

        #endregion

        #region Level 4
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_camera_device_initialize_with_use_case_v210 (HND_TYPE cameraDeviceHnd, [MarshalAs (UnmanagedType.LPStr)] string cameraCaptureModeID);
        #endregion

        #endregion

        #region Enums
        public enum CameraAccessLevel
        {
            L1 = 1, //!< Level 1 access provides the depth data using standard, known-working configurations
            L2 = 2, //!< Level 2 access provides the raw data, for example for developing custom processing pipelines
            L3 = 3, //!< Level 3 access enables you to overwrite exposure limits
            L4 = 4  //!< Level 4 access is for bringing up new camera modules
        };
        #endregion

        //Reference handle ID for native instance
        private HND_TYPE m_cameraDeviceHnd;

        //These listener references are required to prevent their instances from being garbage
        //collected. The framework is unable to track native references.
        private DepthDataListener m_depthDataListener;
        private DepthImageListener m_depthImageListener;
        private ExtendedDataListener m_extendedDataListener;
        private IRImageListener m_irImageListener;
        private SparsePointCloudListener m_sparsePointCloudListener;
        private RecordStoppedListener m_recordStoppedListener;
        // the original ExposureListener uses ExposureListener2 and has no native code
        private ExposureListener2 m_exposureListener;
        private EventListener m_eventListener;

        private List < KeyValuePair<ProcessingFlag, Variant>> m_processingParameters;
        private List<byte> m_calibrationData;

        //Instances of this class are only allowed to be crated through CameraManager
        internal CameraDevice (HND_TYPE cameraDeviceHnd)
        {
            m_cameraDeviceHnd = cameraDeviceHnd;
        }

        /// <summary>
        /// Deletes the CameraDevice instance and releases all allocated resources.
        /// </summary>
        ~CameraDevice()
        {
            royale_camera_device_destroy_v210 (m_cameraDeviceHnd);
        }

        // ----------------------------------------------------------------------------------------------
        // Level 1: Standard users (Laser Class 1 guaranteed)
        // ----------------------------------------------------------------------------------------------
        #region [ACCESS LEVEL 1]

        /// <summary>
        /// LEVEL 1
        /// Initializes the camera device and sets basic settings, this method must be called only once
        /// ideally after the class is created. The settings pointer can also be null which means that a generic
        /// setting is used
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus Initialize ()
        {
            return (CameraStatus) royale_camera_device_initialize_v210 (m_cameraDeviceHnd);
        }

        /// <summary>
        /// LEVEL 1
        /// Get the ID of the camera device
        /// </summary>
        /// <param name="cameraId">output variable for camera id</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetId (out string cameraId)
        {
            IntPtr idPtr;
            CameraStatus status = (CameraStatus) royale_camera_device_get_id_v220 (m_cameraDeviceHnd, out idPtr);

            if (CameraStatus.SUCCESS == status)
            {
                cameraId = Marshal.PtrToStringAnsi (idPtr);
                royale_free_string (idPtr);
            }
            else
            {
                cameraId = null;
            }

            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// the associated camera name as a string which is defined in the CoreConfig of each module.
        /// </summary>
        /// <param name="cameraName">output variable for camera name</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetCameraName (out string cameraName)
        {
            IntPtr namePtr;
            CameraStatus status = (CameraStatus) royale_camera_device_get_camera_name_v220 (m_cameraDeviceHnd, out namePtr);

            if (CameraStatus.SUCCESS == status)
            {
                cameraName = Marshal.PtrToStringAnsi (namePtr);
                royale_free_string (namePtr);
            }
            else
            {
                cameraName = null;
            }

            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// Retrieve further information for this specific camera.
        /// </summary>
        /// <param name="cameraInfo">output variable for map, where the keys are depending on the used camera</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetCameraInfo (out List<KeyValuePair<string, string>> cameraInfo)
        {
            UInt32 numData = 0;
            cameraInfo = new List<KeyValuePair<string, string>>();
            IntPtr infoPtr;
            CameraStatus status = (CameraStatus) royale_camera_device_get_camera_info_v220 (m_cameraDeviceHnd, out infoPtr, ref numData);

            if (CameraStatus.SUCCESS == status && numData > 0)
            {
                IntPtr[] infoPtrArray = new IntPtr[numData * 2];
                Marshal.Copy (infoPtr, infoPtrArray, 0, (int) numData * 2);

                for (int i = 0; i < numData * 2; i += 2)
                {
                    string key = Marshal.PtrToStringAnsi (infoPtrArray[i]);
                    string value = Marshal.PtrToStringAnsi (infoPtrArray[i + 1]);
                    cameraInfo.Add (new KeyValuePair<string, string> (key, value));
                }
                royale_free_pair_string_string_array (out infoPtr, numData);
            }

            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Starts the video capture mode (free-running), based on the specified operation mode.
        /// A listener needs to be registered in order to retrieve the data stream. Either raw data
        /// or processed data can be consumed. If no data listener is registered an error will be returned
        /// and capturing is not started.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus StartCapture()
        {
            return (CameraStatus) royale_camera_device_start_capture_v210 (m_cameraDeviceHnd);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Stops the video capturing mode. All buffers should be released again by the data listener.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus StopCapture()
        {
            return (CameraStatus) royale_camera_device_stop_capture_v210 (m_cameraDeviceHnd);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Once registering the data listener, 3D point cloud data is sent via the callback function.
        /// </summary>
        /// <param name="listener">listener interface which needs to implement the callback method</param>
        public CameraStatus RegisterDepthDataListener (IDepthDataListener listener)
        {
            CameraStatus status = UnregisterDepthDataListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_depthDataListener = new DepthDataListener (listener.OnNewData);
                status = (CameraStatus) royale_camera_device_register_data_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_depthDataListener.nativeCallback));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the data depth listener
        /// </summary>
        public CameraStatus UnregisterDepthDataListener()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_data_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_depthDataListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Once registering the data listener, Android depth image data is sent via the callback function.
        /// </summary>
        /// <param name="listener">listener interface which needs to implement the callback method</param>
        public CameraStatus RegisterDepthImageListener (IDepthImageListener listener)
        {
            CameraStatus status = UnregisterDepthImageListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_depthImageListener = new DepthImageListener (listener.OnNewData);
                status = (CameraStatus) royale_camera_device_register_depth_image_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_depthImageListener.nativeCallback));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the depth image listener
        /// </summary>
        public CameraStatus UnregisterDepthImageListener()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_depth_image_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_depthImageListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Once registering the data listener, Android point cloud data is sent via the callback function.
        /// </summary>
        /// <param name="listener">listener interface which needs to implement the callback method</param>
        public CameraStatus RegisterSparsePointCloudListener (ISparsePointCloudListener listener)
        {
            CameraStatus status = UnregisterSparsePointCloudListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_sparsePointCloudListener = new SparsePointCloudListener (listener.OnNewData);
                status = (CameraStatus) royale_camera_device_register_spc_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_sparsePointCloudListener.nativeCallback));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the sparse point cloud listener listener
        /// </summary>
        public CameraStatus UnregisterSparsePointCloudListener()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_spc_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_sparsePointCloudListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Once registering the data listener, IR image data is sent via the callback function.
        /// </summary>
        /// <param name="listener">listener interface which needs to implement the callback method</param>
        public CameraStatus RegisterIRImageListener (IIRImageListener listener)
        {
            CameraStatus status = UnregisterIRImageListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_irImageListener = new IRImageListener (listener.OnNewData);
                status = (CameraStatus) royale_camera_device_register_ir_image_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_irImageListener.nativeCallback));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the IR image listener listener
        /// </summary>
        public CameraStatus UnregisterIRImageListener()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_ir_image_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_irImageListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Once registering a record listener, the listener gets notified once recording has stopped after
        /// specified frames.
        /// </summary>
        /// <param name="listener">listener interface which needs to implement the callback method</param>
        public CameraStatus RegisterRecordStoppedListener (IRecordStoppedListener listener)
        {
            CameraStatus status = UnregisterRecordStoppedListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_recordStoppedListener = new RecordStoppedListener (listener.OnRecordStopped);
                status = (CameraStatus) royale_camera_device_register_record_stop_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_recordStoppedListener.nativeOnRecordStopped));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the record listener.
        /// </summary>
        public CameraStatus UnregisterRecordStoppedListener()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_record_stop_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_recordStoppedListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// [deprecated]
        /// Once registering the exposure listener, new exposure values calculated by the processing are sent
        /// to the listener.  Every callback will contain only one exposure time, and in mixed mode there
        /// is no indication which stream the callback refers to.  To receive the streamId, use the
        /// IExposureListener2 interface instead.
        ///
        /// The original IExposureListener returns an array of times, but that array is always
        /// only one element, containing the same time that IExposureListener2 returns.  Both
        /// interfaces are only called when autoexposure is active.
        /// </summary>
        /// <param name="listener">interface which needs to implement the callback method</param>
        /// <returns>CameraStatus</returns>
        [System.Obsolete ("IExposureListener is replaced by IExposureListener2")]
        public CameraStatus RegisterExposureListener (IExposureListener listener)
        {
            // ExposureListener is a wrapper implementing the RoyaleDotNet IExposureListener2
            return RegisterExposureListener (new ExposureListener (listener));
        }

        /// <summary>
        /// LEVEL 1
        /// Once registering the exposure listener, new exposure values calculated by the processing are sent
        /// to the listener.
        /// </summary>
        /// <param name="listener">interface which needs to implement the callback method</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus RegisterExposureListener (IExposureListener2 listener)
        {
            CameraStatus status = UnregisterExposureListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_exposureListener = new ExposureListener2 (listener.OnNewExposure);
                status = (CameraStatus) royale_camera_device_register_exposure_listener_stream_v300 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_exposureListener.nativeCallback));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the exposure listener
        /// </summary>
        public CameraStatus UnregisterExposureListener()
        {
            // the CAPI's unreg_v210 matches both reg_v210 and reg_v300
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_exposure_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_exposureListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// Once registering the event listener, new events are sent to the listener.
        /// </summary>
        /// <param name="listener">interface which needs to implement the callback method</param>
        public CameraStatus RegisterEventListener (IEventListener listener)
        {
            CameraStatus status = UnregisterEventListener();
            if (status == CameraStatus.SUCCESS)
            {
                m_eventListener = new EventListener (listener.OnEvent);
                status = (CameraStatus) royale_camera_device_register_event_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_eventListener.nativeEvent));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Unregisters the event listener
        /// </summary>
        public CameraStatus UnregisterEventListener()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_event_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_eventListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Returns the maximal width supported by the camera device.
        /// </summary>
        /// <returns>Maximal width supported by the camera device.</returns>
        public CameraStatus GetMaxSensorWidth (out UInt16 maxSensorWidth)
        {
            return (CameraStatus) royale_camera_device_get_max_sensor_width_v220 (m_cameraDeviceHnd, out maxSensorWidth);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Returns the maximal width supported by the camera device.
        /// </summary>
        /// <returns>Maximal width supported by the camera device.</returns>
        public CameraStatus GetMaxSensorHeight (out UInt16 maxSensorHeight)
        {
            return (CameraStatus) royale_camera_device_get_max_sensor_height_v220 (m_cameraDeviceHnd, out maxSensorHeight);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Sets the use case for the camera. If the use case is supported by the connected camera device
        /// SUCCESS will be returned. Please get the list of supported usecases by calling GetUseCases().
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public CameraStatus SetUseCase (string name)
        {
            return (CameraStatus) royale_camera_device_set_use_case_v210 (m_cameraDeviceHnd, name);
        }

        /// <summary>
        /// LEVEL 1
        /// Get the streams associated with the current use case.
        /// </summary>
        /// <param name="streams">identifiers for each stream</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetStreams (out UInt16[] streams)
        {
            StreamId.NativeVectorStreamId nativeVector = new StreamId.NativeVectorStreamId();
            CameraStatus status = (CameraStatus) royale_camera_device_get_streams_v300 (m_cameraDeviceHnd, ref nativeVector);
            if (status == CameraStatus.SUCCESS)
            {
                streams = StreamId.convertNative (nativeVector);
                royale_free_vector_stream_id (ref nativeVector);
            }
            else
            {
                streams = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// Get the number of streams associated with the given use case.
        /// </summary>
        /// <param name="nrStreams">output variable for the number of streams</param>
        /// <param name="name">use case name</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetNumberOfStreams (out UInt32 nrStreams, string name)
        {
            return (CameraStatus) royale_camera_device_get_number_of_streams_v330 (m_cameraDeviceHnd, name, out nrStreams);
        }

        /// <summary>
        /// LEVEL 1
        /// Returns the current usecase of the camera
        /// </summary>
        /// <param name="currentUseCase">output variable for the current use case</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetCurrentUseCase (out string currentUseCase)
        {
            IntPtr useCasePtr;
            CameraStatus status = (CameraStatus) royale_camera_device_get_current_use_case_v220 (m_cameraDeviceHnd, out useCasePtr);

            if (CameraStatus.SUCCESS == status)
            {
                currentUseCase = Marshal.PtrToStringAnsi (useCasePtr);
                royale_free_string (useCasePtr);
            }
            else
            {
                currentUseCase = null;
            }

            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// Returns all use cases which are supported by the connected module and valid for the current
        /// selected CallbackData information (e.g. Raw, Depth, ...)
        /// </summary>
        /// <param name="useCases">output variable for list of supported use cases</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetUseCases (out List<string> useCases)
        {
            useCases = new List<string>();

            IntPtr useCasesPtr;
            UInt32 numUseCases;
            CameraStatus status = (CameraStatus) royale_camera_device_get_use_cases_v220 (m_cameraDeviceHnd, out useCasesPtr, out numUseCases);

            if (CameraStatus.SUCCESS == status && numUseCases > 0)
            {
                IntPtr[] stringPointers = new IntPtr[numUseCases];
                Marshal.Copy (useCasesPtr, stringPointers, 0, (int) numUseCases);
                foreach (IntPtr stringPtr in stringPointers)
                {
                    useCases.Add (Marshal.PtrToStringAnsi (stringPtr));
                }
                royale_free_string_array2 (ref useCasesPtr, numUseCases);
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Returns the information if the camera is properly initialized and a connection is established.
        /// </summary>
        /// <param name="isConnected">output variable for connected state. Set to true if the camera is properly
        /// initialized and a connection is established, false otherwise.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus IsConnected (out bool isConnected)
        {
            return (CameraStatus) royale_camera_device_is_connected_v220 (m_cameraDeviceHnd, out isConnected);
        }

        /// <summary>
        /// LEVEL 1
        /// Returns the information if the camera is currently in capture mode
        /// </summary>
        /// <param name="isCapturing">output variable for capturing state. Set to true if the camera is currently
        /// in capture mode, false otherwise.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus IsCapturing (out bool isCapturing)
        {
            return (CameraStatus) royale_camera_device_is_capturing_v220 (m_cameraDeviceHnd, out isCapturing);
        }

        /// <summary>
        /// LEVEL 1
        /// Returns the information if the camera module is calibrated. Older camera modules
        /// can still be operated with royale, but calibration data may be incomplete.
        /// </summary>
        /// <param name="isCalibrated">output variable for calibrated state. Set to true if the module contains
        /// proper calibration data.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus IsCalibrated (out bool isCalibrated)
        {
            return (CameraStatus) royale_camera_device_is_calibrated_v220 (m_cameraDeviceHnd, out isCalibrated);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Returns the current camera device access level
        /// </summary>
        /// <param name="accessLevel">output variable for current camera device access level.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetAccessLevel (out CameraAccessLevel accessLevel)
        {
            return (CameraStatus) royale_camera_device_get_access_level_v220 (m_cameraDeviceHnd, out accessLevel);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Change the exposure time for supported operated operation modes. The exposure time can also be altered during capture
        /// mode. The maximal exposure time is defined by the used operation mode.
        /// </summary>
        /// <param name="exposureTime">exposure time in micro seconds</param>
        /// <param name="streamId">for mixed mode use cases, the stream to act on; for
        /// single-stream use cases the default value of 0 (which is otherwise not a valid stream
        /// id) can be used to refer to that stream for backward compatibility</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetExposureTime (UInt32 exposureTime, UInt16 streamId = 0)
        {
            return (CameraStatus) royale_camera_device_set_exposure_time_v300 (m_cameraDeviceHnd, streamId, exposureTime);
        }

        /// <summary>
        /// LEVEL 1
        /// Change the exposure mode for the supported operated operation modes. If MANUAL exposure mode of operation is chosen, the user
        /// is able to determine set exposure time manually within the boundaries of the exposure limits of the specific operation mode.
        /// In AUTOMATIC mode the optimum exposure settings are determined the system itself.
        /// </summary>
        /// <param name="exposureMode">mode of operation to determine the exposure time</param>
        /// <param name="streamId">for mixed mode use cases, the stream to act on; for
        /// single-stream use cases the default value of 0 (which is otherwise not a valid stream
        /// id) can be used to refer to that stream for backward compatibility</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetExposureMode (ExposureMode exposureMode, UInt16 streamId = 0)
        {
            return (CameraStatus) royale_camera_device_set_exposure_mode_v300 (m_cameraDeviceHnd, streamId, (UInt32) exposureMode);
        }

        /// <summary>
        /// LEVEL 1
        /// Retrieves the current mode of operation for acquisition of the exposure time.
        /// </summary>
        /// <param name="exposureMode">output variable for current exposure mode</param>
        /// <param name="streamId">for mixed mode use cases, the stream to act on; for
        /// single-stream use cases the default value of 0 (which is otherwise not a valid stream
        /// id) can be used to refer to that stream for backward compatibility</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetExposureMode (out ExposureMode exposureMode, UInt16 streamId = 0)
        {
            return (CameraStatus) royale_camera_device_get_exposure_mode_v300 (m_cameraDeviceHnd, streamId, out exposureMode);
        }

        /// <summary>
        /// LEVEL 1
        /// Set the frame rate to a value. Upper bound is given by the use case.
        /// E.g. Usecase with 5 FPS, a frame rate of maximal 5 can be set.
        /// This function is not supported for mixed-mode.
        /// </summary>
        /// <param name="frameRate">Requested frame rate.</param>
        /// <returns></returns>
        public CameraStatus SetFrameRate (UInt16 frameRate)
        {
            return (CameraStatus) royale_camera_device_set_frame_rate_v210 (m_cameraDeviceHnd, frameRate);
        }

        /// <summary>
        /// LEVEL 1
        /// Get the current frame rate which is set for the current use case.
        /// This function is not supported for mixed-mode.
        /// </summary>
        /// <param name="frameRate">output variable for current frame rate.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetFrameRate (out UInt16 frameRate)
        {
            return (CameraStatus) royale_camera_device_get_frame_rate_v220 (m_cameraDeviceHnd, out frameRate);
        }

        /// <summary>
        /// LEVEL 1
        /// Get the maximal frame rate which can be set for the current use case.
        /// This function is not supported for mixed-mode.
        /// </summary>
        /// <param name="maxFrameRate">output variable for maximum frame rate.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetMaxFrameRate (out UInt16 maxFrameRate)
        {
            return (CameraStatus) royale_camera_device_get_max_frame_rate_v220 (m_cameraDeviceHnd, out maxFrameRate);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Retrieves the minimum and maximum allowed exposure limits of the specified operation mode.
        /// Can be used to retrieve the allowed operational range for a manual definition of the exposure time.
        /// </summary>
        /// <param name="limits">output variable for minimum and maximum allowed exposure limits of the
        /// specified operation mode.</param>
        /// <param name="streamId">for mixed mode use cases, the stream to act on; for
        /// single-stream use cases the default value of 0 (which is otherwise not a valid stream
        /// id) can be used to refer to that stream for backward compatibility</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetExposureLimits (out ExposureLimits limits, UInt16 streamId = 0)
        {
            UInt32 lowerLimit = UInt32.MaxValue;
            UInt32 upperLimit = 0;
            CameraStatus status = (CameraStatus) royale_camera_device_get_exposure_limits_v300 (m_cameraDeviceHnd, streamId, out lowerLimit, out upperLimit);

            limits = new ExposureLimits (lowerLimit, upperLimit);

            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Gets the intrinsics of the camera module which are stored in the calibration file.
        /// </summary>
        /// <param name="lensParams">output variable for lens parameters storing all relevant
        /// information (c,f,p,k), null on error.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetLensParameters (out LensParameters lensParams)
        {
            LensParameters.NativeLensParameters nativeParams = new LensParameters.NativeLensParameters();
            CameraStatus status = (CameraStatus) royale_camera_device_get_lens_parameters_v210 (m_cameraDeviceHnd, ref nativeParams);
            if (CameraStatus.SUCCESS == status)
            {
                lensParams = new LensParameters (nativeParams);
                royale_free_lens_parameters (ref nativeParams);
            }
            else
            {
                lensParams = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Start recording the raw data stream into a file. The recording will capture
        /// the raw data coming from the imager.
        /// </summary>
        /// <param name="fileName">Full path of target filename (proposed suffix is .rrf)</param>
        /// <param name="numberOfFrames">Maximal number of frames which should be captured (stop will be called
        ///     automatically). If zero (default) is set, recording will happen till StopRecording() is called.</param>
        /// <param name="framesSkip">indicate how many frames should be skipped after every recorded frame. If zero (default)
        ///     is set and msSkip is zero, every frame will be recorded.</param>
        /// <param name="msSkip">indicate how many milliseconds should be skipped after every recorded frame. If zero (default)
        ///     is set and frameSkip is zero, every frame will be recorded.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus StartRecording (string fileName, UInt32 numberOfFrames, UInt32 framesSkip, UInt32 msSkip)
        {
            return (CameraStatus) royale_camera_device_start_recording_v210 (m_cameraDeviceHnd, fileName, numberOfFrames, framesSkip, msSkip);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Stop recording the raw data stream into a file. After the recording is stopped
        /// the file is available on the file system.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus StopRecording()
        {
            return (CameraStatus) royale_camera_device_stop_recording_v210 (m_cameraDeviceHnd);
        }

        /// <summary>
        /// LEVEL 1
        /// <para/>Enables/disables the external trigger
        /// </summary>
        /// <param name="useExternalTrigger"> true if the external trigger should be used, false otherwise</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetExternalTrigger (bool useExternalTrigger)
        {
            return (CameraStatus) royale_camera_device_set_external_trigger_v330 (m_cameraDeviceHnd, useExternalTrigger);
        }

        #endregion

        // ----------------------------------------------------------------------------------------------
        // Level 2: Experienced users (Laser Class 1 guaranteed) - activation key required
        // ----------------------------------------------------------------------------------------------
        #region [ACCESS LEVEL 2]

        /// <summary>
        /// LEVEL 2
        /// Get the list of exposure groups supported by the currently set use case.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetExposureGroups (out List<String> exposureGroups)
        {
            exposureGroups = new List<string>();

            IntPtr groupsPtr;
            UInt32 numGroups;
            CameraStatus status = (CameraStatus) royale_camera_device_get_exposure_groups_v300 (m_cameraDeviceHnd, out groupsPtr, out numGroups);

            if (CameraStatus.SUCCESS == status && numGroups > 0)
            {
                IntPtr[] stringPointers = new IntPtr[numGroups];
                Marshal.Copy (groupsPtr, stringPointers, 0, (int) numGroups);
                foreach (IntPtr stringPtr in stringPointers)
                {
                    exposureGroups.Add (Marshal.PtrToStringAnsi (stringPtr));
                }
                royale_free_string_array2 (ref groupsPtr, numGroups);
            }
            return status;
        }


        /// <summary>
        /// LEVEL 2
        /// Change the exposure time for the supported operated operation modes. If MANUAL exposure
        /// mode of operation is chosen, the user is able to determine set exposure time manually
        /// within the boundaries of the exposure limits of the specific operation mode.
        /// <para/>In any other mode of operation the method will return EXPOSURE_MODE_INVALID to
        /// indicate incompatibility with the selected exposure mode. If the CameraDevice is the
        /// playback of a recording then LOGIC_ERROR is returned instead.
        /// </summary>
        /// <param name="exposureGroup">exposure group to be updated</param>
        /// <param name="exposureTime">exposure time in microseconds</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetExposureTime (String exposureGroup, UInt32 exposureTime)
        {
            return (CameraStatus) royale_camera_device_set_group_exposure_time_v300 (m_cameraDeviceHnd, exposureGroup, exposureTime);
        }

        /// <summary>
        /// LEVEL 2
        /// Retrieves the minimum and maximum allowed exposure limits of the specified operation mode.
        /// Limits may vary between exposure groups.
        /// Can be used to retrieve the allowed operational range for a manual definition of the
        /// exposure time.
        /// </summary>
        /// <param name="exposureGroup">exposure group to be queried</param>
        /// <param name="exposureLimits">pair of (minimum, maximum) exposure time in microseconds</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetExposureLimits (String exposureGroup, out ExposureLimits exposureLimits)
        {
            UInt32 lowerLimit = UInt32.MaxValue;
            UInt32 upperLimit = 0;
            CameraStatus status = (CameraStatus) royale_camera_device_get_group_exposure_limits_v300 (m_cameraDeviceHnd, exposureGroup, out lowerLimit, out upperLimit);

            exposureLimits = new ExposureLimits (lowerLimit, upperLimit);

            return status;

        }

        /// <summary>
        /// LEVEL 2
        /// Change the exposure times for all exposure groups. If the vector that is provided is too
        /// long the extraneous values will be discarded. If the vector is too short an error will
        /// be returned.
        /// </summary>
        /// <param name="exposureTimes">vector with exposure times in microseconds</param>
        /// <param name="streamId">for mixed mode use cases, the stream to act on; for
        /// single-stream use cases the default value of 0 (which is otherwise not a valid stream
        /// id) can be used to refer to that stream for backward compatibility</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetExposureTimes (List<UInt32> exposureTimes, UInt16 streamId = 0)
        {
            return (CameraStatus) royale_camera_device_set_exposure_times_v300 (m_cameraDeviceHnd, streamId, exposureTimes.ToArray(), (UInt32) exposureTimes.Count);
        }

        /// <summary>
        /// LEVEL 2
        /// Change the exposure times for all exposure groups.
        /// The order of the exposure times is aligned with the order of exposure groups received by getExposureGroups.
        /// If the vector that is provided is too long the extraneous values will be discard.
        /// If the vector is too short an error will be returned.
        /// </summary>
        /// <param name="exposureTimes">vector with exposure times in microseconds</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus setExposureForGroups (List<UInt32> exposureTimes)
        {
            return (CameraStatus) royale_camera_device_set_exposure_for_groups_v300 (m_cameraDeviceHnd, exposureTimes.ToArray(), (UInt32) exposureTimes.Count);
        }

        /// <summary>
        /// LEVEL 2
        /// Set/alter processing parameters in order to control the data output. A list of processing flags
        /// is available as an enumeration. The `Variant` data type can take float, int, or bool. Please
        /// make sure to set the proper `Variant` type for the enum.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetProcessingParameters (KeyValuePair<ProcessingFlag, Variant>[] processingParameters, UInt16 streamId = 0)
        {
            UInt32[] parameters = new UInt32[processingParameters.Length * 3];
            UInt32 i = 0;
            foreach (KeyValuePair<ProcessingFlag, Variant> entry in processingParameters)
            {
                parameters[i++] = (UInt32) entry.Key;
                parameters[i++] = (UInt32) entry.Value.Type;
                parameters[i++] = (UInt32) entry.Value.getData();
            }
            return (CameraStatus) royale_camera_device_managed_set_proc_params (m_cameraDeviceHnd, streamId, parameters, (UInt32) parameters.Length);
        }

        /// <summary>
        /// LEVEL 2
        /// Retrieve the available processing parameters which are used for the calculation.
        /// </summary>
        /// <param name="processingParameters">output variable for processing parameter map</param>
        /// <param name="streamId">which stream's parameters to get</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetProcessingParameters (out List<KeyValuePair<ProcessingFlag, Variant>> processingParameters, UInt16 streamId = 0)
        {
            m_processingParameters = null;
            CameraStatus status = (CameraStatus) royale_camera_device_managed_get_proc_params (m_cameraDeviceHnd, streamId,
                                  Marshal.GetFunctionPointerForDelegate (new GetProcessingParameterNativeCallback (
                                              GetProcessingParameterCallback)));

            processingParameters = m_processingParameters;

            return status;
        }

        internal void GetProcessingParameterCallback (IntPtr data, UInt32 totalLength)
        {
            int[] parameterArray = new int[totalLength];
            Marshal.Copy (data, parameterArray, 0, (int) totalLength);

            m_processingParameters = new List<KeyValuePair<ProcessingFlag, Variant>>();

            for (int i = 0; i < totalLength;)
            {
                ProcessingFlag flag = (ProcessingFlag) parameterArray[i++];
                Variant.VariantType type = (Variant.VariantType) parameterArray[i++];
                Variant variant = new Variant (type, (UInt32) parameterArray[i++]);
                m_processingParameters.Add (new KeyValuePair < ProcessingFlag, Variant > (flag, variant));
            }
        }

        /// <summary>
        /// LEVEL 2
        /// Once registering extended data listener, extended data is sent via the callback function.
        /// If depth data only is specified, this listener is not called. For this case, please use
        /// the standard depth data listener.
        /// </summary>
        /// <param name="listener">interface which needs to implement the callback method</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus RegisterDataListenerExtended (IExtendedDataListener listener)
        {
            CameraStatus status = UnregisterDataListenerExtended();
            if (status == CameraStatus.SUCCESS)
            {
                m_extendedDataListener = new ExtendedDataListener (listener.OnNewData);
                status = (CameraStatus) royale_camera_device_register_extended_data_listener_v210 (m_cameraDeviceHnd,
                         Marshal.GetFunctionPointerForDelegate (m_extendedDataListener.nativeCallback));
            }
            return status;
        }

        /// <summary>
        /// LEVEL 2
        /// Unregisters the data extended listener.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus UnregisterDataListenerExtended()
        {
            CameraStatus status = (CameraStatus) royale_camera_device_unregister_extended_data_listener_v210 (m_cameraDeviceHnd);
            if (status == CameraStatus.SUCCESS)
            {
                m_extendedDataListener = null;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 2
        /// [deprecated]
        /// Set the callback output data type to one type only.
        ///
        /// INFO: This method needs to be called before startCapture(). If is is called during the camera
        /// is in capture mode, it will only have effect after the next stop/start sequence.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetCallbackData (UInt16 cbData)
        {
            return (CameraStatus) royale_camera_device_set_callback_data_v210 (m_cameraDeviceHnd, cbData);
        }

        /// <summary>
        /// LEVEL 2
        /// Set the callback output data type to one type only.
        ///
        /// INFO: This method needs to be called before startCapture(). If is is called during the camera
        /// is in capture mode, it will only have effect after the next stop/start sequence.
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetCallbackData (CallbackData cbData)
        {
            return (CameraStatus) royale_camera_device_set_callback_data_v210 (m_cameraDeviceHnd, (UInt16) cbData);
        }

        /// <summary>
        /// LEVEL 2
        /// Loads a different calibration from a file. This calibration data will also be used
        /// by the processing!
        /// </summary>
        /// <param name="calibrationFileName">filename name of the calibration file which should be loaded.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetCalibrationData (string calibrationFileName)
        {
            return (CameraStatus) royale_camera_device_set_calibration_data_file (m_cameraDeviceHnd, calibrationFileName);
        }

        /// <summary>
        /// LEVEL 2
        /// Loads a different calibration from a given Vector.This calibration data will also be used
        /// by the processing!
        /// </summary>
        /// <param name="calibrationData">data calibration data which should be used.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetCalibrationData (List<byte> calibrationData)
        {
            IntPtr unmanagedPointer = Marshal.AllocHGlobal (calibrationData.Count);
            Marshal.Copy (calibrationData.ToArray(), 0, unmanagedPointer, calibrationData.Count);
            CameraStatus status = (CameraStatus) royale_camera_device_set_calibration_data_v210 (m_cameraDeviceHnd, ref unmanagedPointer,
                                  (UInt32) calibrationData.Count);
            Marshal.FreeHGlobal (unmanagedPointer);

            return status;
        }

        /// <summary>
        /// LEVEL 2
        /// Retrieves the current calibration data.
        /// </summary>
        /// <param name="calibrationData">output variable for calibration data.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetCalibrationData (out List<byte> calibrationData)
        {
            m_calibrationData = null;
            CameraStatus status = (CameraStatus) royale_camera_device_managed_get_calib_data (m_cameraDeviceHnd,
                                  Marshal.GetFunctionPointerForDelegate (new GetGetCalibrationDataNativeCallback (
                                              GetCalibrationCallback)));

            calibrationData = m_calibrationData;

            return status;
        }

        internal void GetCalibrationCallback (IntPtr data, UInt32 totalLength)
        {
            m_calibrationData = new List<byte>();

            byte[] calibrationArray = new byte[totalLength];
            Marshal.Copy (data, calibrationArray, 0, (int) totalLength);

            m_calibrationData.AddRange (calibrationArray);
        }

        /// <summary>
        /// LEVEL 2
        /// Tries to write the current calibration file into the internal flash of the device.
        /// If no flash is found RESOURCE_ERROR is returned. If there are errors during the flash
        /// process it will try to restore the original calibration.
        ///
        /// This is not yet implemented for all cameras!
        /// </summary>
        /// <returns>CameraStatus</returns>
        public CameraStatus WriteCalibrationToFlash()
        {
            return (CameraStatus) royale_camera_device_write_calibration_to_flash_v210 (m_cameraDeviceHnd);
        }
        #endregion

        // ----------------------------------------------------------------------------------------------
        // Level 3: Advanced users (Laser Class 1 not (!) guaranteed) - activation key required
        // ----------------------------------------------------------------------------------------------
        #region [ACCESS LEVEL 3]

        /// <summary>
        /// LEVEL 3
        /// Change the dutycycle of a certain sequence. If the dutycycle is not supported,</summary>
        /// an error will be returned. The dutycycle can also be altered during capture
        /// mode.
        /// <param name="dutyCycle">dutyCycle in percent (0, 100)</param>
        /// <param name="index">index of the sequence to change</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus SetDutyCycle (double dutyCycle, UInt16 index)
        {
            return (CameraStatus) royale_camera_device_set_duty_cycle_v210 (m_cameraDeviceHnd, dutyCycle, index);
        }

        /// <summary>
        /// LEVEL 3
        /// For each element of the vector a single register write is issued for the connected imager.
        /// </summary>
        /// <param name="registers">
        ///  Contains elements of possibly not-unique (String, uint64_t) duplets.
        ///  The String component can consist of:
        ///     a) a base-10 decimal number in the range of [0, 65535]
        ///     b) a base-16 hexadecimal number preceeded by a "0x" in the range of [0, 65535]
        /// </param>
        /// <returns>CameraStatus</returns>
        public CameraStatus WriteRegisters (List<KeyValuePair<string, UInt64>> registers)
        {
            NativePairStringUInt64[] nativeRegisters = new NativePairStringUInt64[registers.Count];
            IntPtr[] pointers = new IntPtr[registers.Count];
            int i = 0;

            foreach (KeyValuePair<string, UInt64> entry in registers)
            {
                nativeRegisters[i].first = entry.Key;
                nativeRegisters[i].second = entry.Value;

                pointers[i] = Marshal.AllocHGlobal (Marshal.SizeOf (nativeRegisters[i]));
                Marshal.StructureToPtr (nativeRegisters[i], pointers[i], false);
                i++;
            }

            CameraStatus status = (CameraStatus) royale_camera_device_write_registers_v210 (m_cameraDeviceHnd, pointers, (UInt32) registers.Count);

            foreach (IntPtr pointer in pointers)
            {
                Marshal.FreeHGlobal (pointer);
            }

            return status;
        }

        /// <summary>
        /// LEVEL 3
        /// For each element of the vector a single register read is issued for the connected imager.
        /// </summary>
        /// <param name="registers">
        ///  Contains elements of possibly not-unique (String, uint64_t) duplets.
        ///  The String component can consist of:
        ///     a) a base-10 decimal number in the range of [0, 65535]
        ///     b) a base-16 hexadecimal number preceeded by a "0x" in the range of [0, 65535]
        ///  The values of these will be overwritten with the values read from the device.
        /// </param>
        /// <returns>CameraStatus</returns>
        public CameraStatus ReadRegisters (ref List<KeyValuePair<string, UInt64>> registers)
        {
            NativePairStringUInt64[] nativeRegisters = new NativePairStringUInt64[registers.Count];
            IntPtr[] pointers = new IntPtr[registers.Count];

            int i = 0;
            foreach (KeyValuePair<string, UInt64> entry in registers)
            {
                nativeRegisters[i].first = entry.Key;
                nativeRegisters[i].second = entry.Value;

                pointers[i] = Marshal.AllocHGlobal (Marshal.SizeOf (nativeRegisters[i]));
                Marshal.StructureToPtr (nativeRegisters[i], pointers[i], false);
                i++;
            }

            CameraStatus status = (CameraStatus) royale_camera_device_read_registers_v210 (m_cameraDeviceHnd, pointers, (UInt32) registers.Count);

            registers.Clear();

            foreach (IntPtr pointer in pointers)
            {
                NativePairStringUInt64 currentRegister = (NativePairStringUInt64) Marshal.PtrToStructure (pointer, typeof (NativePairStringUInt64));
                registers.Add (new KeyValuePair<string, UInt64> (currentRegister.first, currentRegister.second));
                Marshal.FreeHGlobal (pointer);
            }

            return status;
        }

        /// <summary>
        /// LEVEL 3
        /// Shift the current lens center by the given translation.
        /// If the resulting lens center is not valid this function will return an error.
        /// This function works only for raw data readout.</summary>
        /// <param name="tx">translation in x direction</param>
        /// <param name="ty">translation in y direction</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus ShiftLensCenter (Int16 tx, Int16 ty)
        {
            return (CameraStatus) royale_camera_device_shift_lens_center_v320 (m_cameraDeviceHnd, tx, ty);
        }

        /// <summary>
        /// LEVEL 3
        /// Retrieves the current lens center.</summary>
        /// <param name="x">current x center</param>
        /// <param name="y">current y center</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus GetLensCenter (out UInt16 x, out UInt16 y)
        {
            UInt16 cx, cy;
            CameraStatus status = (CameraStatus) royale_camera_device_get_lens_center_v320 (m_cameraDeviceHnd, out cx, out cy);

            if (status == CameraStatus.SUCCESS)
            {
                x = cx;
                y = cy;
            }
            else
            {
                x = 0;
                y = 0;
            }
            return status;
        }

        /// <summary>
        /// LEVEL 3
        /// Writes an arbitrary vector of data on to the storage of the device.
        /// </summary>
        /// <param name="flashData">data which should be used.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus WriteDataToFlash (List<byte> flashData)
        {
            IntPtr unmanagedPointer = Marshal.AllocHGlobal (flashData.Count);
            Marshal.Copy (flashData.ToArray(), 0, unmanagedPointer, flashData.Count);
            CameraStatus status = (CameraStatus) royale_camera_device_write_data_to_flash_v31000 (m_cameraDeviceHnd, ref unmanagedPointer,
                                  (UInt32) flashData.Count);
            Marshal.FreeHGlobal (unmanagedPointer);

            return status;
        }

        /// <summary>
        /// LEVEL 3
        /// Writes an arbitrary file on to the storage of the device.
        /// </summary>
        /// <param name="fileName">file which should be used.</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus WriteDataToFlash (string fileName)
        {
            return (CameraStatus) royale_camera_device_write_data_to_flash_file_v31000 (m_cameraDeviceHnd, fileName);
        }

        #endregion

        // ----------------------------------------------------------------------------------------------
        // Level 4: Direct imager access (Laser Class 1 not (!) guaranteed) - activation key required
        // ----------------------------------------------------------------------------------------------
        #region [ACCESS LEVEL 4]

        /// <summary>
        /// LEVEL 4
        /// Initialize the camera and configure the system for the specified use case
        /// </summary>
        /// <param name="initialUseCase">identifies the use case by an case sensitive string</param>
        /// <returns>CameraStatus</returns>
        public CameraStatus InitializeWithUseCase (string initialUseCase)
        {
            return (CameraStatus) royale_camera_device_initialize_with_use_case_v210 (m_cameraDeviceHnd, initialUseCase);

        }
        #endregion
    }
}
