using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using RoyaleDotNet;
using System.Collections.Generic;
using System.Threading;

//\cond HIDDEN_SYMBOLS
namespace test_RoyaleDotNet
{
    [TestClass]
    public class TestCameraDeviceL2
    {
        CameraStatus status;
        CameraManager cameraManager;
        CameraDevice cameraDevice;

        TestExtendedDataListener exDataListener;

        bool hasBeenCalled;

        static bool shouldHaveDepth = true;
        static bool shouldHaveRaw = true;
        static bool shouldHaveIntermediate = true;

        static object listenerCalledLock;

        static int timeOutForListenerCallback = 2000;

        static ExtendedData receivedData;

        [TestInitialize()]
        public void Initialize()
        {
            listenerCalledLock = new object();
        }

        [TestCleanup()]
        public void Cleanup()
        {
            listenerCalledLock = null;
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
            GC.WaitForPendingFinalizers(); // needed to run consecutive tests
        }

        [TestMethod]
        public void TestLevel2()
        {
            exDataListener = new TestExtendedDataListener();
            Assert.IsNotNull (exDataListener, "Failed to create ExtendedDataListener.");

            CheckSetupAndStart();
            CheckExposure();
            CheckExposureGroups();
            CheckProcessingParameters();
            CheckExtendedDataListener();
            CheckCalibrationData();
            CheckStopCapture();
        }

        private void CheckCalibrationData()
        {
            // We don't test writing calibration, because an automated test is not expected to
            // overwrite data on the device.
            // status = cameraDevice.WriteCalibrationToFlash();
            // Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to write calibration data to flash.");

            List<byte> calibrationData;
            status = cameraDevice.GetCalibrationData (out calibrationData);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetCalibrationData call failed.");
            Assert.IsNotNull (calibrationData, "Unable to get calibration data.");

            status = cameraDevice.SetCalibrationData (calibrationData);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set calibration data.");
        }

        private void CheckExtendedDataListener()
        {
            status = cameraDevice.RegisterDataListenerExtended (exDataListener);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to register ExtendedDataListener.");

            // for later use when setCallbackData works as expected
            /*
            for (int i = 1; i < 8; i++)
            {
                cameraDevice.StopCapture();
                CallbackData dataType = CallbackData.None;

                shouldHaveDepth = (i & 0x1) > 0;
                shouldHaveRaw = (i & 0x2) > 0;
                shouldHaveIntermediate = (i & 0x4) > 0;

                if (shouldHaveDepth)
                {
                    dataType |= CallbackData.Depth;
                }

                if (shouldHaveRaw)
                {
                    dataType |= CallbackData.Raw;
                }

                if (shouldHaveIntermediate)
                {
                    dataType |= CallbackData.Intermediate;
                }

                status = cameraDevice.SetCallbackData (dataType);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set CallbackDataType");

                Thread.Sleep (1000);
            */
            lock (listenerCalledLock)
            {
                //cameraDevice.StartCapture();
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);

                Assert.IsTrue (hasBeenCalled, "ExtendedDatatlistener has not been called.");

                Assert.IsNotNull (receivedData, "Received Data is null");

                bool hasDepthData = receivedData.HasDepthData;
                bool hasRawData = receivedData.HasRawData;
                bool hasIntermediatedata = receivedData.HasIntermediateData;

                Assert.AreEqual (shouldHaveDepth, hasDepthData,
                                 "Data " + getHasString (hasDepthData) +
                                 " DeptData but it " + getShouldString (shouldHaveDepth));

                Assert.AreEqual (shouldHaveRaw, hasRawData,
                                 "Data " + getHasString (hasRawData) +
                                 " RawData but it " + getShouldString (shouldHaveRaw));

                Assert.AreEqual (shouldHaveIntermediate, hasIntermediatedata,
                                 "Data " + getHasString (hasIntermediatedata) +
                                 " IntermediatData but it " + getShouldString (shouldHaveIntermediate));

                if (hasDepthData)
                {
                    DepthData dd = receivedData.DepthData;
                    Assert.IsNotNull (dd, "Received DepthData is invalid.");
                    Assert.IsTrue (dd.exposureTimes.Length > 0, "Received DepthData.ExposureTimes is invalid.");
                    Assert.IsTrue (dd.points.Length > 0, "Received DepthData.Points is invalid.");
                    Assert.IsTrue (dd.width > 0, "Received DepthData.Width is invalid.");
                    Assert.IsTrue (dd.height > 0, "Received DepthData.Height is invalid.");
                    Assert.IsTrue (dd.version > 0, "Received DepthData.Version is invalid.");
                    Assert.IsTrue (dd.timeStamp > 0, "Received DepthData.TimeStamp is invalid.");
                }

                if (hasRawData)
                {
                    RawData rd = receivedData.RawData;
                    Assert.IsNotNull (rd, "Received RawData is invalid.");
                    Assert.IsTrue (rd.exposureTimes.Length > 0, "Received RawData.ExposureTimes is invalid.");
                    Assert.IsTrue (rd.rawData.Count > 0, "Received RawData.RawData is invalid.");
                    Assert.IsTrue (rd.modulationFrequencies.Length > 0, "Received RawData.ModulationFrequencies is invalid.");
                    Assert.IsTrue (rd.width > 0, "Received RawData.Width is invalid.");
                    Assert.IsTrue (rd.height > 0, "Received RawData.Height is invalid.");
                    Assert.IsTrue (rd.illuminationTemperature > 0, "Received RawData.IlluminationTemperature is invalid.");
                    Assert.IsTrue (rd.timeStamp > 0, "Received RawData.TimeStamp is invalid.");
                    Assert.IsTrue (rd.phaseAngles.Length > 0, "Received RawData.phaseAngles is invalid.");
                    Assert.IsTrue (rd.illuminationEnabled.Length > 0, "Received RawData.illuminationEnabled is invalid.");
                }

                if (hasIntermediatedata)
                {
                    IntermediateData id = receivedData.IntermediateData;
                    Assert.IsNotNull (id, "Received IntermediateData is invalid.");
                    Assert.IsTrue (id.exposureTimes.Length > 0, "Received IntermediateData.ExposureTimes is invalid.");
                    Assert.IsTrue (id.numFrequencies > 0, "Received IntermediateData.NumFrequencies is invalid.");
                    Assert.IsTrue (id.modulationFrequencies.Length > 0, "Received IntermediateData.ModulationFrequencies is invalid.");
                    Assert.IsTrue (id.width > 0, "Received IntermediateData.Width is invalid.");
                    Assert.IsTrue (id.height > 0, "Received IntermediateData.Height is invalid.");
                    Assert.IsTrue (id.points.Length > 0, "Received IntermediateData.Points is invalid.");
                    Assert.IsTrue (id.timeStamp > 0, "Received IntermediateData.TimeStamp is invalid.");
                    Assert.IsTrue (id.version > 0, "Received IntermediateData.Version is invalid.");
                }
            }
            //}

            //cameraDevice.StartCapture();

            status = cameraDevice.UnregisterDataListenerExtended();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to unregister ExtendedDatalistener.");

            lock (listenerCalledLock)
            {
                hasBeenCalled = Monitor.Wait (listenerCalledLock, timeOutForListenerCallback);
            }
            Assert.IsFalse (hasBeenCalled, "ExtendedDatalistener has been called after unregister.");
        }

        private void CheckStopCapture()
        {
            status = cameraDevice.StopCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to stop capturing.");

            bool isCapturing;
            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "isCapturing call failed.");
            Assert.IsFalse (isCapturing, "CameraDevice is still capturing.");
        }

        private void CheckSetupAndStart()
        {
            cameraManager = new CameraManager(AccessCodes.ROYALE_ACCESS_CODE_LEVEL2);
            Assert.IsNotNull (cameraManager, "Failed to create CameraManager.");

            List<string> connectedCameras = cameraManager.GetConnectedCameraList();
            Assert.IsTrue (connectedCameras.Count > 0, "No connected CameraDevice found.");

            cameraDevice = cameraManager.CreateCamera (connectedCameras[0]);
            Assert.IsNotNull (cameraDevice, "Failed to create CameraDevice.");

            CameraDevice.CameraAccessLevel accessLevel;
            status = cameraDevice.GetAccessLevel (out accessLevel);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get AccessLevel.");
            Assert.AreEqual (CameraDevice.CameraAccessLevel.L2, accessLevel, "CameraDevice has wrong access level.");

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

            status = cameraDevice.StartCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to start capturing.");

            bool isCapturing;
            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "isCapturing call failed.");
            Assert.IsTrue (isCapturing, "CameraDevice is not capturing.");
        }

        private void CheckProcessingParameters()
        {
            List<KeyValuePair<ProcessingFlag, Variant>> pParameters;
            status = cameraDevice.GetProcessingParameters (out pParameters);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetProcessingParameters call failed.");
            Assert.IsTrue (pParameters != null, "Failed to get ProcessingParameters.");
            Assert.IsTrue (pParameters.Count > 0, "Failed to get ProcessingParameters.");

            status = cameraDevice.SetProcessingParameters (pParameters.ToArray());
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set ProcessingParameters.");
        }

        private void CheckExposure()
        {
            ExposureLimits limits;
            status = cameraDevice.GetExposureLimits (out limits);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get exposure limits.");

            List<UInt32> exposureList = new List<uint>();
            for (int i = 0; i < 9; i++)
            {
                exposureList.Add (limits.LowerLimit);
            }

            for (int i = 0; i < 10; i++)
            {
                status = cameraDevice.SetExposureTimes (exposureList);
                if (status != CameraStatus.DEVICE_IS_BUSY)
                {
                    break;
                }
                Thread.Sleep (500);
            }
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set all exposure times at once.");
        }

        private void CheckExposureForGroups()
        {
            ExposureLimits limits;
            status = cameraDevice.GetExposureLimits (out limits);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get exposure limits.");

            List<UInt32> exposureList = new List<uint>();
            for (int i = 0; i < 9; i++)
            {
                exposureList.Add (limits.LowerLimit);
            }

            for (int i = 0; i < 10; i++)
            {
                status = cameraDevice.setExposureForGroups (exposureList);
                if (status != CameraStatus.DEVICE_IS_BUSY)
                {
                    break;
                }
                Thread.Sleep (500);
            }
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set all exposure times at once.");
        }

        // based on the CAPI test_level2_exposure_groups
        private void CheckExposureGroups()
        {
            List<string> groups;
            status = cameraDevice.GetExposureGroups (out groups);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get exposure groups");
            Assert.AreNotEqual (0, groups.Count, "No exposure groups");

            for (int i = 0; i < groups.Count; i++)
            {
                string group = groups[i];
                Assert.IsNotNull (group, "Null group name");
                Assert.AreNotEqual (0, group.Length, "Empty group name");

                // get exposure time to limits
                ExposureLimits limits;
                status = cameraDevice.GetExposureLimits (group, out limits);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get group's exposure limits");
                Assert.IsNotNull (limits, "Failed to get ExposureLimits.");
                Assert.IsTrue (limits.LowerLimit <= limits.UpperLimit, "Bad exposure limits");
                Assert.IsTrue (limits.UpperLimit < 10000, "Corrupt exposure limits?"); //arbitrary limit, but not expected to fail

                // set exposure time to limits
                Thread.Sleep (2000);
                status = cameraDevice.SetExposureTime (group, limits.LowerLimit);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set exposure time to lower limit");

                Thread.Sleep (2000);
                status = cameraDevice.SetExposureTime (group, limits.UpperLimit);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to set exposure time to uppec limit");
            }
        }

        private string getHasString (bool has)
        {
            return has ? "has" : "has no";
        }

        private string getShouldString (bool should)
        {
            return should ? "should" : "shouldn't";
        }

        internal class TestExtendedDataListener : IExtendedDataListener
        {
            public void OnNewData (ExtendedData data)
            {
                lock (listenerCalledLock)
                {
                    receivedData = data;
                    Monitor.Pulse (listenerCalledLock);
                }
            }
        }
    }
}
//\endcond
