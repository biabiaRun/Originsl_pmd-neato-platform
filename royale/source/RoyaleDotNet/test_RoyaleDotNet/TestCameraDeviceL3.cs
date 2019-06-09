using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using RoyaleDotNet;
using System.Collections.Generic;
using System.Threading;

//\cond HIDDEN_SYMBOLS
namespace test_RoyaleDotNet
{
    [TestClass]
    public class TestCameraDeviceL3
    {
        CameraStatus status;
        CameraManager cameraManager;
        CameraDevice cameraDevice;

        [TestInitialize()]
        public void Initialize()
        {
        }

        [TestCleanup()]
        public void Cleanup()
        {
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

        private void CheckSetupAndStart()
        {
            cameraManager = new CameraManager(AccessCodes.ROYALE_ACCESS_CODE_LEVEL3);
            Assert.IsNotNull (cameraManager, "Failed to create CameraManager.");

            List<string> connectedCameras = cameraManager.GetConnectedCameraList();
            Assert.IsTrue (connectedCameras.Count > 0, "No connected CameraDevice found.");

            cameraDevice = cameraManager.CreateCamera (connectedCameras[0]);
            Assert.IsNotNull (cameraDevice, "Failed to create CameraDevice.");

            CameraDevice.CameraAccessLevel accessLevel;
            status = cameraDevice.GetAccessLevel (out accessLevel);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get AccessLevel.");
            Assert.AreEqual (CameraDevice.CameraAccessLevel.L3, accessLevel, "CameraDevice has wrong access level.");

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
        }

        [TestMethod]
        public void TestLevel3WriteReadRegisters()
        {
            CheckSetupAndStart();

            // On a pico flexx, this is a write-and-read test, using a register which is known to be
            // ok to use on that device. On any other device, it's a read-only test.
            string cameraName;
            status = cameraDevice.GetCameraName (out cameraName);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetCameraName call failed.");
            bool okToWrite = cameraName == "PICOFLEXX";

            List<KeyValuePair<string, UInt64>> writeRegisters = new List<KeyValuePair<string, UInt64>>();
            writeRegisters.Add (new KeyValuePair<string, UInt64> ("0xb06e", 0x2124));
            writeRegisters.Add (new KeyValuePair<string, UInt64> ("0xb06e", 0x0300));

            if (okToWrite)
            {
                status = cameraDevice.WriteRegisters (writeRegisters);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to write registers");
            }

            List<KeyValuePair<string, UInt64>> readRegisters = new List<KeyValuePair<string, UInt64>>();
            readRegisters.Add (new KeyValuePair<string, UInt64> ("0xb06e", 0));
            readRegisters.Add (new KeyValuePair<string, UInt64> ("0xb06e", UInt64.MaxValue));

            status = cameraDevice.ReadRegisters (ref readRegisters);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to read registers");
            Assert.AreEqual (2, readRegisters.Count);

            if (okToWrite)
            {
                //same register has been written, compare to last write
                KeyValuePair<string, UInt64> expected = writeRegisters[1];

                for (int i = 0; i < readRegisters.Count; i++)
                {
                    KeyValuePair<string, UInt64> actual = readRegisters[i];

                    Assert.AreEqual (expected.Key, actual.Key);
                    Assert.AreEqual (expected.Value, actual.Value);
                }
            }
            else
            {
                Assert.AreEqual (readRegisters[0].Key, readRegisters[1].Key,
                                 "Test assumption (not system under test) failed: expected to read the same register twice");
                Assert.AreEqual (readRegisters[0].Value, readRegisters[1].Value,
                                 "Read different values for the same register");
            }
        }

        [TestMethod]
        public void TestLevel3()
        {
            CheckSetupAndStart();

            status = cameraDevice.SetDutyCycle (50.0, 0);
            Assert.IsTrue (CameraStatus.SUCCESS == status || CameraStatus.DUTYCYCLE_NOT_SUPPORTED == status,
                           "Failed to set the duty cycle, and unexpected error status");

            status = cameraDevice.StartCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to start capturing.");

            bool isCapturing;
            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsCapturing call failed.");
            Assert.IsTrue (isCapturing, "CameraDevice is not capturing.");

            status = cameraDevice.StopCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to stop capturing.");

            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsCapturing call failed.");
            Assert.IsFalse (isCapturing, "CameraDevice is still capturing.");
        }

        // \todo: make the tests below similar to the C++ or CAPI ones
        [TestMethod]
        public void TestLevel3MixedModeStreamIds()
        {
            CheckSetupAndStart();

            status = cameraDevice.SetUseCase ("MODE_MIXED");
            if (status != CameraStatus.SUCCESS)
            {
                // no mixed mode support
                return;
            }

            UInt16[] streams;
            status = cameraDevice.GetStreams (out streams);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Couldn't get the stream ids");
            Assert.IsNotNull (streams, "GetStreams returned null");
            Assert.AreEqual (2, streams.Length, "Unexpected number of streams");
            Assert.AreEqual (0xdefa, streams[0], "Not the expected default stream ID");
            Assert.AreEqual (0xdefb, streams[1], "Not the expected second stream ID");
        }
    }
}
//\endcond
