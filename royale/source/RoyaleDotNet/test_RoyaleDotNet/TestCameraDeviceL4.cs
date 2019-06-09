using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using RoyaleDotNet;
using System.Collections.Generic;
using System.Threading;

//\cond HIDDEN_SYMBOLS
namespace test_RoyaleDotNet
{
    [TestClass]
    public class TestCameraDeviceL4
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

        [TestMethod]
        public void TestLevel4()
        {
            cameraManager = new CameraManager(AccessCodes.ROYALE_ACCESS_CODE_LEVEL4);
            Assert.IsNotNull (cameraManager, "Failed to create CameraManager.");

            List<string> connectedCameras = cameraManager.GetConnectedCameraList();
            Assert.IsTrue (connectedCameras.Count > 0, "No connected CameraDevice found.");

            cameraDevice = cameraManager.CreateCamera (connectedCameras[0]);
            Assert.IsNotNull (cameraDevice, "Failed to create CameraDevice.");

            CameraDevice.CameraAccessLevel accessLevel;
            status = cameraDevice.GetAccessLevel (out accessLevel);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to get AccessLevel.");
            Assert.AreEqual (CameraDevice.CameraAccessLevel.L4, accessLevel, "CameraDevice has wrong access level.");

            string cameraID;
            status = cameraDevice.GetId (out cameraID);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetId call failed.");
            Assert.AreEqual (connectedCameras[0], cameraID, "Camera ID is not matching.");

            string cameraName;
            status = cameraDevice.GetCameraName (out cameraName);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "GetCameraName call failed.");
            string useCase = null;
            if (cameraName == "PICOFLEXX")
            {
                useCase = "MODE_9_5FPS_2000";
            }
            else if (cameraName == "PICOMAXX1" ||
                     cameraName == "PICOMAXX2" ||
                     cameraName == "PICOMONSTAR1" ||
                     cameraName == "PICOMONSTAR2")
            {
                useCase = "MODE_9_5FPS_1900";
            }

            if (useCase != null)
            {

                status = cameraDevice.InitializeWithUseCase (useCase);
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to initialize CameraDevice with UseCase " + useCase);
            }
            else
            {
                status = cameraDevice.Initialize ();
                Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to initialize CameraDevice with default UseCase");
            }

            bool isConnected;
            status = cameraDevice.IsConnected (out isConnected);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsConnectedcall failed.");
            Assert.IsTrue (isConnected, "CameraDevice is not connected.");

            status = cameraDevice.StartCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to start capturing.");

            bool isCapturing;
            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsCapturing call failed.");
            Assert.IsTrue (isCapturing, "CameraDevice is not capturing.");

            status = cameraDevice.StopCapture();
            Assert.AreEqual (CameraStatus.SUCCESS, status, "Failed to stop capturing.");

            status = cameraDevice.IsCapturing (out isCapturing);
            Assert.AreEqual (CameraStatus.SUCCESS, status, "IsCapturing call failed."); ;
            Assert.IsFalse (isCapturing, "CameraDevice is still capturing.");
        }
    }
}
//\endcond
