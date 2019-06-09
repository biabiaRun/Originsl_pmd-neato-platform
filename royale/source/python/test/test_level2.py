#!/usr/bin/python3

import roypy
import unittest
import time
import os

class MyExtendedListener(roypy.IExtendedDataListener):
    def __init__(self):
        super(MyExtendedListener, self).__init__()
        self.hasRawData = False
        self.hasIntermediateData = False
        self.hasDepthData = False

    def onNewData(self, data):
        if data.hasRawData():
            self.hasRawData = True
        if data.hasIntermediateData():
            self.hasIntermediateData = True
        if data.hasDepthData():
            self.hasDepthData = True
    
class TestLevel2(unittest.TestCase):
    def setUp(self):
        print("Test : ", self._testMethodName)
        c = roypy.CameraManager("${ROYALE_ACCESS_CODE_LEVEL2}")
        l = c.getConnectedCameraList()

        if l.size() == 0:
            raise RuntimeError ("No cameras connected")
        
        id = l[0]
        self.cam = c.createCamera(id)

    def tearDown(self):
        self.cam = None

    def testInitialize(self):
        self.cam.initialize()
        level = self.cam.getAccessLevel()
        self.assertEqual(level, 2)

    def testRawCallback(self):
        self.cam.initialize()
        l = MyExtendedListener()
        self.cam.registerDataListenerExtended(l)
        self.cam.startCapture()
        time.sleep(0.5)
        self.cam.stopCapture()
        self.assertTrue(l.hasRawData)
        self.assertTrue(l.hasIntermediateData)
        self.assertTrue(l.hasDepthData)
        
if __name__ == "__main__": 
    unittest.main()