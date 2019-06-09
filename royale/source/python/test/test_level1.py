#!/usr/bin/python3

import roypy
import unittest
import time
import os

class MyDepthDataListener(roypy.IDepthDataListener):
    def __init__(self):
        super(MyDepthDataListener, self).__init__()
        self.receivedData = False

    def onNewData(self, data):
        self.receivedData = True
    
class MyRecordStopListener(roypy.IRecordStopListener):
    def __init__(self):
        super(MyRecordStopListener, self).__init__()
        self.receivedStop = False

    def onRecordingStopped(self, frameCount):
        self.receivedStop = True
    
class TestLevel1(unittest.TestCase):
    def setUp(self):
        print("Test : ", self._testMethodName)
        c = roypy.CameraManager()
        l = c.getConnectedCameraList()

        if l.size() == 0:
            raise RuntimeError ("No cameras connected")
        
        id = l[0]
        self.cam = c.createCamera(id)

    def tearDown(self):
        self.cam = None

    def testInitialize(self):
        self.cam.initialize()

    def testStartStop(self):
        self.cam.initialize()
        self.cam.startCapture()
        time.sleep(0.5)
        self.cam.stopCapture()

    def testExposureTimes(self):
        self.cam.initialize()
        self.cam.startCapture()
        time.sleep(0.5)
        self.cam.setExposureTime(200)
        time.sleep(0.5)
        self.cam.stopCapture()

    def testRecordingTimed(self):
        """A recording where the test application stops it after a given time"""
        if os.path.exists("test_record_py.rrf"):
            os.remove("test_record_py.rrf")
        self.cam.initialize()
        l = MyRecordStopListener()
        self.cam.registerRecordListener(l)
        self.cam.startCapture()
        self.cam.startRecording("test_record_py.rrf")
        time.sleep(1)
        self.assertFalse(l.receivedStop)
        self.cam.stopRecording()
        self.cam.stopCapture()
        self.assertTrue(os.path.exists("test_record_py.rrf"))

    def testRecordingCounted(self):
        """A recording where Royale stops it after a given number of frames"""
        if os.path.exists("test_record_py.rrf"):
            os.remove("test_record_py.rrf")
        self.cam.initialize()
        l = MyRecordStopListener()
        self.cam.registerRecordListener(l)
        self.cam.startCapture()
        self.cam.startRecording("test_record_py.rrf", 1)
        time.sleep(1)
        self.assertTrue(l.receivedStop)
        self.cam.stopCapture()
        self.assertTrue(os.path.exists("test_record_py.rrf"))

    def testDepthCallback(self):
        self.cam.initialize()
        l = MyDepthDataListener()
        self.cam.registerDataListener(l)
        self.cam.startCapture()
        time.sleep(0.5)
        self.cam.setExposureTime(200)
        time.sleep(0.5)
        self.cam.stopCapture()
        self.assertTrue(l.receivedData)

    def testUseCases(self):
        self.cam.initialize()
        useCases = self.cam.getUseCases()
        self.cam.startCapture()
        for u in range(useCases.size()):
            time.sleep(0.5)
            self.cam.setUseCase(useCases[u])
            numStreams = self.cam.getNumberOfStreams(useCases[u])
            if (numStreams < 1):
                raise RuntimeError ("Wrong number of streams")
        self.cam.stopCapture()

    def testVersion(self):
        a,b,c,d = roypy.getVersion()
        verString = roypy.getVersionString()
        verString2 = str(a)+'.'+str(b)+'.'+str(c)+'.'+str(d)
        if verString != verString2:
            raise RuntimeError ("Wrong version string")
        print(verString)

    def testExpoLimits(self):
        self.cam.initialize()
        self.cam.startCapture()
        time.sleep(0.5)
        limits = self.cam.getExposureLimits()
        self.cam.stopCapture()

    def testCameraInfo(self):
        self.cam.initialize()
        self.cam.startCapture()
        time.sleep(0.5)
        camInfo = self.cam.getCameraInfo()
        self.cam.stopCapture()

    def testFilterLevel(self):
        self.cam.initialize()
        self.cam.setFilterLevel(roypy.FilterLevel_Off, 0)
        filterLevel = self.cam.getFilterLevel(0)
        self.assertEqual(filterLevel, roypy.FilterLevel_Off)
        
if __name__ == "__main__": 
    unittest.main()
