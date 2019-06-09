using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using RoyaleDotNet;
using System.Collections.Generic;
using System.Threading;
using System.IO;

//\cond HIDDEN_SYMBOLS
namespace test_RoyaleDotNet
{
    [TestClass]
    public class TestCameraDeviceL1
    {
        CameraStatus status;
        CameraManager cameraManager;
        CameraDevice cameraDevice;
        TestDepthDataListener ddListener;
        TestRecordStoppedListener rsListener;
        TestExposureListener1 exListener1;
        TestExposureListener2 exListener2;
        TestDepthImageListener diListener;
        TestSparsePointCloudListener spcListener;
        TestIRImageListener irListener;
        TestEventListener evListener;

        bool hasBeenCalled;

        /// <summary>
        /// Pulsed when any listener except the exposure listener receives a callback.
        /// </summary>
        static object listenerCalledLock;
        /// <summary>
        /// A separate listenerCalledLock for the exposure listeners (because their tests
        /// require an imager data listener to be registered at the same time).
        /// </summary>
        static object exposureCalledLock;

        static uint numFramesToRecord = 2;
        static int timeOutForListenerCallback = 2000;
        static string recordFile = "test1.rec";

        static DepthData receivedData;
        static DepthImage receivedImage;
        static SparsePointCloud receivedSPC;
        static IRImage receivedIRImage;
        static int numFramesRecorded = 0;


        [TestInitialize()]
        public void Initialize()
        {
            listenerCalledLock = new object();
            exposureCalledLock = new object();
        }

        [TestCleanup()]
        public void Cleanup()
        {
            listenerCalledLock = null;
            exposureCalledLock = null;

            if (cameraDevice != null)
            {
                bool isConnected;
                bool isCapturing;

                cameraDevice.IsConnected (out isConnected);
                cameraDevice.IsCapturing (out isCapturing);

                if (isConnected && isCapturing)
                {
                    cameraDevice.StopCapture();
                }
                cameraDevice = null;
            }

            cameraManager = null;
            GC.Collect (GC.MaxGeneration);
            GC.WaitForPendingFinalizers(); // ROYAL-2656 needed to run consecutive tests
        }

        [TestMethod]
        public void TestLevel1()
        {
            CheckCreateListeners();
            CheckSetupAndInfo();
            CheckStartCapture();
            CheckFrameRate();
            CheckDepthDataListener();
            CheckDepthImageListener();
            CheckSparsePointCloudListener();
            CheckIRImageListener();
            CheckRecordStoppedListener();
            CheckEventListener();
            CheckUseCases();
            CheckStreams();
            CheckExposureTimes();
            CheckExposureCallback();
            CheckLensParameters();
            CheckStopCapture();
        }

        private void CheckLensParameters()
        {
            LensParameters lensParameters;
            status = cameraDevice.GetLensParameters (out lensParameters);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetLensParameters call failed.");
            Assert.IsNotNull (lensParameters, "Failed to get LensParameters.");
        }

        private void CheckCreateListeners()
        {
            ddListener = new TestDepthDataListener();
            Assert.IsNotNull (ddListener, "Failed to create DepthDataListener.");

            rsListener = new TestRecordStoppedListener();
            Assert.IsNotNull (rsListener, "Failed to create RecordStoppedListener.");

            exListener1 = new TestExposureListener1();
            Assert.IsNotNull (exListener1, "Failed to create ExposureListener (1).");

            exListener2 = new TestExposureListener2();
            Assert.IsNotNull (exListener2, "Failed to create ExposureListener2.");

            diListener = new TestDepthImageListener();
            Assert.IsNotNull (diListener, "Failed to create DepthImageListener.");

            spcListener = new TestSparsePointCloudListener();
            Assert.IsNotNull (spcListener, "Failed to create SparsePointCloudListener.");

            irListener = new TestIRImageListener();
            Assert.IsNotNull (irListener, "Failed to create IRImageListener.");

            evListener = new TestEventListener();
            Assert.IsNotNull (evListener, "Failed to create EventListener");
        }

        private void CheckStartCapture()
        {
            status = cameraDevice.StartCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to start capturing.");

            bool isCapturing;
            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "isCapturing call failed.");
            Assert.IsTrue (isCapturing, "CameraDevice is not capturing.");
        }

        private void CheckStopCapture ()
        {
            status = cameraDevice.StopCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to stop capturing.");

            bool isCapturing;
            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "isCapturing call failed.");
            Assert.IsFalse (isCapturing, "CameraDevice is still capturing.");
        }

        private void CheckSetupAndInfo()
        {
            cameraManager = new CameraManager();
            Assert.IsNotNull (cameraManager, "Failed to create CameraManager.");

            List<string> connectedCameras = cameraManager.GetConnectedCameraList();
            Assert.IsTrue (connectedCameras.Count > 0, "No connected CameraDevice found.");

            cameraDevice = cameraManager.CreateCamera (connectedCameras[0]);
            Assert.IsNotNull (cameraDevice, "Failed to create CameraDevice.");

            CameraDevice.CameraAccessLevel accessLevel;
            status = cameraDevice.GetAccessLevel (out accessLevel);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get AccessLevel.");
            Assert.AreEqual (CameraDevice.CameraAccessLevel.L1, accessLevel, "CameraDevice has wrong access level.");

            string cameraName;
            status = cameraDevice.GetCameraName (out cameraName);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetCameraName call failed.");
            Assert.IsFalse (String.IsNullOrEmpty (cameraName));

            string cameraID;
            status = cameraDevice.GetId (out cameraID);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetId call failed.");
            Assert.AreEqual (connectedCameras[0], cameraID, "Camera ID is not matching.");

            status = cameraDevice.Initialize();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to initialize CameraDevice.");

            bool isConnected;
            status = cameraDevice.IsConnected (out isConnected);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsConnected call failed.");
            Assert.IsTrue (isConnected, "CameraDevice is not connected.");

            bool isCalibrated;
            status = cameraDevice.IsCalibrated (out isCalibrated);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsCalibrated call failed.");
            Assert.IsTrue (isCalibrated, "CameraDevice is not calibrated.");

            UInt16 maxWidth;
            status = cameraDevice.GetMaxSensorWidth (out maxWidth);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetMaxSensorWidth call failed.");
            Assert.IsTrue (maxWidth > 0, "Unable to get MaxSensorWidth.");

            UInt16 maxHeight;
            status = cameraDevice.GetMaxSensorHeight (out maxHeight);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetMaxSensorHeight call failed.");
            Assert.IsTrue (maxHeight > 0, "Unable to get MaxSensorHeight.");

            List<KeyValuePair<string, string>> cameraInfo;
            status = cameraDevice.GetCameraInfo (out cameraInfo);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetCameraInfo call failed.");
            Assert.IsNotNull (cameraInfo);
        }

        private void CheckUseCases()
        {
            List<string> useCases;
            status = cameraDevice.GetUseCases (out useCases);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetUseCases call failed.");
            Assert.IsTrue (useCases.Count > 0, "No UseCase for the selected CameraDevice found.");

            string targetUseCase = useCases[useCases.Count - 1];
            status = cameraDevice.SetUseCase (targetUseCase);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change UseCase.");

            string currentUseCase;
            status = cameraDevice.GetCurrentUseCase (out currentUseCase);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetCurrentUseCase call failed.");
            Assert.AreEqual (targetUseCase, currentUseCase, "Set UseCase differs from targeted UseCase.");

            // reset to first use case
            targetUseCase = useCases[0];
            status = cameraDevice.SetUseCase(targetUseCase);
            Assert.AreEqual(CameraStatus.SUCCESS, status, "Failed to change UseCase.");
        }

        private void CheckStreams()
        {
            UInt16[] streams;
            status = cameraDevice.GetStreams (out streams);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetStreams call failed.");
            Assert.IsTrue (streams.Length > 0, "No streams in the current use case");
            Assert.AreEqual(0xdefa, streams[0], "Not the expected default stream id");
        }

        private void CheckFrameRate()
        {
            UInt16 maxFPS;
            status = cameraDevice.GetMaxFrameRate (out maxFPS);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetMaxFrameRate call failed.");
            Assert.AreNotEqual (0, maxFPS, "Unable to get max frame rate");

            status = cameraDevice.SetFrameRate (maxFPS);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set frame rate.");

            UInt16 frameRate;
            status = cameraDevice.GetFrameRate (out frameRate);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetFrameRate call failed.");
            Assert.AreEqual (frameRate, maxFPS, "Wrong frame rate set.");
        }

        private void CheckExposureTimes()
        {
#pragma warning disable 618
            status = cameraDevice.RegisterExposureListener (exListener1);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register ExposureListener (1).");
#pragma warning restore 618

            status = cameraDevice.RegisterExposureListener (exListener2);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register ExposureListener2.");

            status = cameraDevice.SetExposureMode (ExposureMode.Automatic);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureMode to Automatic.");

            ExposureMode exposureMode;
            status = cameraDevice.GetExposureMode (out exposureMode);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to getExposureMode");
            Assert.AreEqual (ExposureMode.Automatic, exposureMode, "ExposureMode was not set to Automatic.");

            status = cameraDevice.SetExposureMode (ExposureMode.Manual);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureMode to Manual.");

            status = cameraDevice.GetExposureMode (out exposureMode);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to getExposureMode");
            Assert.AreEqual (ExposureMode.Manual, exposureMode, "ExposureMode was not set to Manual.");

            status = cameraDevice.UnregisterExposureListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister ExposureListener.");

            ExposureLimits limits;
            status = cameraDevice.GetExposureLimits (out limits);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetExposureLimits call failed.");
            Assert.IsNotNull (limits, "Failed to get ExposureLimits.");
            Assert.IsTrue (limits.LowerLimit <= limits.UpperLimit, "Bad exposure limits");
            Assert.IsTrue (limits.UpperLimit < 10000, "Corrupt exposure limits?"); //arbitrary limit, but not expected to fail

            status = cameraDevice.SetExposureTime (limits.LowerLimit);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureTime to lower limit.");

            System.Threading.Thread.Sleep (2000);

            status = cameraDevice.SetExposureTime (limits.UpperLimit);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureTime to upper limit.");

            // Wait so that the device is not busy for the next test
            System.Threading.Thread.Sleep (2000);
        }

        /// <summary>
        /// Convenience method called multiple times in TriggerExposureCallback
        ///
        /// If changing to MANUAL mode, this function will block before returning, so that the
        /// caller can immediately call royale_camera_device_set_exposure_time without getting a
        /// BUSY error.
        ///
        /// Prerequisite: the depth imager listener is registered and expecting callbacks
        /// </summary>
        /// <param name="targetMode">Whether to set to MANUAL or AUTOMATIC</param>
        private void SetAndCheckExposureMode (ExposureMode targetMode)
        {
            status = cameraDevice.SetExposureMode (targetMode);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureMode to " + targetMode);

            ExposureMode exposureMode;
            status = cameraDevice.GetExposureMode (out exposureMode);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to getExposureMode");
            Assert.AreEqual (targetMode, exposureMode, "ExposureMode was not set to " + targetMode);

            if (targetMode == ExposureMode.Manual)
            {
                // Assuming the use case is not a mixed-mode one, worst case should be 3 frames:
                // one received frame already in C++ and CAPI callback layer,
                // one in the processing,
                // and one before the imager's safe reconfig happens.
                lock (listenerCalledLock)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        bool hasBeenCalled = Monitor.Wait (listenerCalledLock, 2000);
                        Assert.IsTrue (hasBeenCalled, "No/few callbacks after setting the exposure mode");
                    }
                }
            }
        }

        /// <summary>
        /// When this function is called, there will be at least one callback to the exposure listener
        /// </summary>
        /// <param name="listener">The listener which can be tested to see if HasBeenCalled()</param>
        private void TriggerExposureCallback (ITestCallbackListener listener)
        {
            ExposureLimits exposureLimits;
            status = cameraDevice.GetExposureLimits (out exposureLimits);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetExposureLimits call failed.");
            Assert.IsNotNull (exposureLimits, "Failed to get ExposureLimits.");
            Assert.AreNotEqual (exposureLimits.LowerLimit, exposureLimits.UpperLimit, "Use case has a fixed exposure");

            // Callbacks only happen when the autoexposure changes the exposure.  Set exposure time to
            // one limit, give time for the autoexposure to happen, set the exposure to the other limit
            // and give time for the autoexposure to happen.  Then check there was at least one
            // callback.
            //
            // But to set the exposure without getting a BUSY error, the exposure needs to be in MANUAL
            // mode.
            //
            // Also the autoexposure only runs when data is being processed, which only happens when a
            // data listener is registered.
            status = cameraDevice.RegisterDepthImageListener (diListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register DepthImageListener.");

            SetAndCheckExposureMode (ExposureMode.Manual);
            status = cameraDevice.SetExposureTime (exposureLimits.LowerLimit);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureTime to lower limit.");
            SetAndCheckExposureMode (ExposureMode.Automatic);

            // The callback may happening during this sleep
            lock (exposureCalledLock)
            {
                if (!listener.HasBeenCalled())
                {
                    Monitor.Wait (exposureCalledLock, 2000);
                }
            }

            SetAndCheckExposureMode (ExposureMode.Manual);
            status = cameraDevice.SetExposureTime (exposureLimits.UpperLimit);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to change ExposureTime to upper limit.");
            SetAndCheckExposureMode (ExposureMode.Automatic);

            // The callback may happening during this sleep
            lock (exposureCalledLock)
            {
                if (!listener.HasBeenCalled())
                {
                    Monitor.Wait (exposureCalledLock, 2000);
                }
            }

            // Set back to manual exposure so that it's as expected for other tests. Must be done
            // before unregistering the depth listener (prerequisite for SetAndCheckExposureMode).
            SetAndCheckExposureMode (ExposureMode.Manual);

            status = cameraDevice.UnregisterDepthImageListener ();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister DepthImageListener to upper limit.");
        }

        private void CheckExposureCallback()
        {
            // IExposureListener (1)
            lock (exposureCalledLock)
            {
                exListener1.hasBeenCalled = false;
                exListener1.lastExposureTime = 0;
            }

#pragma warning disable 618
            status = cameraDevice.RegisterExposureListener(exListener1);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register ExposureListener (1).");
#pragma warning restore 618

            TriggerExposureCallback (exListener1);

            lock (exposureCalledLock)
            {
                Assert.IsTrue (exListener1.hasBeenCalled, "Did not receive an IExposureListener (1) callback");
                Assert.AreNotEqual (0, exListener1.lastExposureTime);
            }

            status = cameraDevice.UnregisterExposureListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister ExposureListener.");

            // IExposureListener2
            lock (exposureCalledLock)
            {
                exListener2.hasBeenCalled = false;
                exListener2.lastExposureTime = 0;
                exListener2.lastStreamId = 0;
            }

            status = cameraDevice.RegisterExposureListener (exListener2);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register ExposureListener2.");

            TriggerExposureCallback (exListener2);

            lock (exposureCalledLock)
            {
                Assert.IsTrue (exListener2.hasBeenCalled, "Did not receive an IExposureListener2 callback");
                Assert.AreNotEqual (0, exListener2.lastExposureTime);
                Assert.AreNotEqual (0, exListener2.lastStreamId);
            }

            status = cameraDevice.UnregisterExposureListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister ExposureListener.");
        }

        private void CheckRecordStoppedListener()
        {
            status = cameraDevice.RegisterRecordStoppedListener (rsListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register RecordStoppedListener.");

            lock (listenerCalledLock)
            {
                status = cameraDevice.StartRecording (recordFile, numFramesToRecord, 0, 0);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to start recording.");

                hasBeenCalled = Monitor.Wait (listenerCalledLock, (int) numFramesToRecord * timeOutForListenerCallback);

                Assert.IsTrue (hasBeenCalled, "RecordStoppedListener has not been called.");
                Assert.IsTrue (File.Exists (recordFile), "Record file has not been created.");

                Assert.IsTrue (numFramesRecorded == numFramesToRecord, "Number of recorded frames is wrong.");
            }

            status = cameraDevice.UnregisterRecordStoppedListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister RecordStoppedListener.");

            lock (listenerCalledLock)
            {
                status = cameraDevice.StartRecording (recordFile, numFramesToRecord, 0, 0);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to start recording.");

                hasBeenCalled = Monitor.Wait (listenerCalledLock, (int) numFramesToRecord * timeOutForListenerCallback);
            }
            Assert.IsFalse (hasBeenCalled, "RecordStoppedListener has been called after unregister.");
        }

        private void CheckDepthDataListener()
        {
            status = cameraDevice.RegisterDepthDataListener (ddListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register DepthDataListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
                Assert.IsTrue (hasBeenCalled, "DepthDataListener has not been called.");

                Assert.IsNotNull (receivedData, "Received DepthData is null.");
                Assert.IsTrue (receivedData.exposureTimes.Length > 0, "Received ExposureTimes are invalid.");
                Assert.IsTrue (receivedData.points.Length > 0, "Received Points are invalid.");
                Assert.IsTrue (receivedData.width > 0, "Received width is invalid.");
                Assert.IsTrue (receivedData.height > 0, "Received height is invalid.");
                Assert.IsTrue (receivedData.timeStamp > 0, "Received timeStamp is invalid.");
            }

            status = cameraDevice.UnregisterDepthDataListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister DepthDataListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
            }
            Assert.IsFalse (hasBeenCalled, "DepthDataListener has been called after unregister.");
        }

        private void CheckDepthImageListener()
        {
            status = cameraDevice.RegisterDepthImageListener (diListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register DepthImageListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
                Assert.IsTrue (hasBeenCalled, "DepthImageListener has not been called.");

                Assert.IsNotNull (receivedImage, "Received DepthImage is null.");
                Assert.IsTrue (receivedImage.cdData.Length > 0, "Received Data is invalid.");
                Assert.IsTrue (receivedImage.width > 0, "Received width is invalid.");
                Assert.IsTrue (receivedImage.height > 0, "Received height is invalid.");
                Assert.IsTrue (receivedImage.timeStamp > 0, "Received timeStamp is invalid.");
            }

            status = cameraDevice.UnregisterDepthImageListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister DepthDataListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
            }
            Assert.IsFalse (hasBeenCalled, "DepthImageListener has been called after unregister.");
        }

        private void CheckSparsePointCloudListener()
        {
            status = cameraDevice.RegisterSparsePointCloudListener (spcListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register SparesPointCloudListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
                Assert.IsTrue (hasBeenCalled, "SparesPointCloudListener has not been called.");

                Assert.IsNotNull (receivedSPC, "Received SparsePointCloud is null.");
                Assert.IsTrue (receivedSPC.numPoints > 0, "Received data is invalid.");
                Assert.IsTrue (receivedSPC.xyzcPoints.Length == receivedSPC.numPoints * 4, "Received number of points is invalid");
                Assert.IsTrue (receivedSPC.timeStamp > 0, "Received timeStamp is invalid.");
            }

            status = cameraDevice.UnregisterSparsePointCloudListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister SparesPointCloudListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
            }
            Assert.IsFalse (hasBeenCalled, "SparesPointCloudListener has been called after unregister.");
        }

        private void CheckIRImageListener()
        {
            status = cameraDevice.RegisterIRImageListener (irListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register IRImageListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
                Assert.IsTrue (hasBeenCalled, "DepthImageListener has not been called.");

                Assert.IsNotNull (receivedIRImage, "Received IRImage is null.");
                Assert.IsTrue (receivedIRImage.data.Length > 0, "Received data is invalid.");
                Assert.IsTrue (receivedIRImage.width > 0, "Received width is invalid.");
                Assert.IsTrue (receivedIRImage.height > 0, "Received height is invalid.");
                Assert.IsTrue (receivedIRImage.timeStamp > 0, "Received timeStamp is invalid.");
            }

            status = cameraDevice.UnregisterIRImageListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister IRImageListener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
            }
            Assert.IsFalse (hasBeenCalled, "IRImageListener has been called after unregister.");
        }

        private void CheckEventListener()
        {
            status = cameraDevice.RegisterEventListener (evListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register EventListener.");

            status = cameraDevice.UnregisterEventListener();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister EventListener.");
        }

        private interface ITestCallbackListener
        {
            /// <summary>
            /// In some tests the callbacks may stop after some time, for example when the
            /// autoexposure has settled, so the test can't simply call Monitor.Wait() and expect
            /// the listener to call Pulse(). This returns true if the callback has already
            /// happened.
            ///
            /// The caller should hold the appropriate lock.
            /// </summary>
            bool HasBeenCalled();
        }

        private class TestExposureListener1 : ITestCallbackListener, IExposureListener
        {
            public void OnNewExposure (UInt32[] exposureTimes)
            {
                Assert.AreEqual (1, exposureTimes.Length);
                lock (exposureCalledLock)
                {
                    hasBeenCalled = true;
                    lastExposureTime = exposureTimes[0];
                    Monitor.Pulse (exposureCalledLock);
                }
            }

            public bool HasBeenCalled()
            {
                return hasBeenCalled;
            }

            public bool hasBeenCalled = false;
            public UInt32 lastExposureTime = 0;
        }

        private class TestExposureListener2 : ITestCallbackListener, IExposureListener2
        {
            public void OnNewExposure (UInt32 exposureTime, UInt16 streamId)
            {
                lock (exposureCalledLock)
                {
                    hasBeenCalled = true;
                    lastExposureTime = exposureTime;
                    lastStreamId = streamId;
                    Monitor.Pulse (exposureCalledLock);
                }
            }

            public bool HasBeenCalled()
            {
                return hasBeenCalled;
            }

            public bool hasBeenCalled = false;
            public UInt32 lastExposureTime = 0;
            public UInt16 lastStreamId = 0;
        }

        private class TestDepthDataListener : IDepthDataListener
        {

            public void OnNewData (DepthData data)
            {
                lock (listenerCalledLock)
                {
                    receivedData = data;
                    Monitor.Pulse (listenerCalledLock);
                }
            }
        }

        public class TestRecordStoppedListener : IRecordStoppedListener
        {
            public void OnRecordStopped (uint numFrames)
            {
                lock (listenerCalledLock)
                {
                    numFramesRecorded = (int) numFrames;
                    Monitor.Pulse (listenerCalledLock);
                }
            }
        }

        public class TestDepthImageListener : IDepthImageListener
        {

            public void OnNewData (DepthImage data)
            {
                lock (listenerCalledLock)
                {
                    receivedImage = data;
                    Monitor.Pulse (listenerCalledLock);
                }
            }
        }

        public class TestSparsePointCloudListener : ISparsePointCloudListener
        {
            public void OnNewData (SparsePointCloud data)
            {
                lock (listenerCalledLock)
                {
                    receivedSPC = data;
                    Monitor.Pulse (listenerCalledLock);
                }
            }
        }

        public class TestIRImageListener : IIRImageListener
        {
            public void OnNewData (IRImage data)
            {
                lock (listenerCalledLock)
                {
                    receivedIRImage = data;
                    Monitor.Pulse (listenerCalledLock);
                }
            }
        }

        public class TestEventListener : IEventListener
        {
            public void OnEvent (Event royaleEvent)
            {
                //do nothing, hard to test
            }
        }
    }
}
//\endcond
