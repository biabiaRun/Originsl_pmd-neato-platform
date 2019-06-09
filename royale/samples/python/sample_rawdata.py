#!/usr/bin/python3

# Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""This sample shows how to shows how to capture raw data (which requires level 2 access).

It uses Python's numpy and matplotlib to process and display the data.
"""

import argparse
import roypy
import time
import queue
import math
from sample_camera_info import print_camera_info
from roypy_sample_utils import CameraOpener, add_camera_opener_options
from roypy_platform_utils import PlatformHelper

import numpy as np
import matplotlib.pyplot as plt

class MyListener(roypy.IExtendedDataListener):
    def __init__(self, q):
        super(MyListener, self).__init__()
        self.queue = q

    def onNewData(self, data):
        # find out if the callback contains raw data
        if data.hasRawData():
            # retrieve the raw data
            rawData = data.getRawData()

            print("New frame")
            print(rawData.rawFrameCount)
            print(rawData.modulationFrequencies)
            print(rawData.phaseAngles)

            rfc = 0
            for i in range(0, len(rawData.rawFrameCount)):
                rfc = rfc + rawData.rawFrameCount[i]

            phases = []
            for i in range(0, rfc):
                # for retrieving the phase data from the rawData object
                # you should use the special function that is provided
                phaseData = rawData.getRawPhase(i)
                phaseArray = np.asarray(phaseData)
                p = phaseArray.reshape (-1, rawData.width)
                phases.append(p)

            # put the data into the queue so that it can be
            # consumed in the main thread
            self.queue.put(phases)

    def paint (self, data):
        """Called in the main thread, with data containing one of the items that was added to the
        queue in onNewData.
        """
        # create a figure and show the raw data
        plt.figure(1)
        nrows = math.ceil(len(data) / 3.)
        for i in range(0, len(data)):
            plt.subplot(nrows, 3, i+1)
            plt.imshow(data[i])

        plt.show(block = False)
        plt.draw()
        # this pause is needed to ensure the drawing for
        # some backends
        plt.pause(0.001)

def main ():
    platformhelper = PlatformHelper()
    parser = argparse.ArgumentParser (usage = __doc__)
    add_camera_opener_options (parser)
    parser.add_argument ("--seconds", type=int, default=15, help="duration to capture data")
    options = parser.parse_args()
    opener = CameraOpener (options, min_access_level=2)
    cam = opener.open_camera ()

    print_camera_info (cam)

    # we will use this queue to synchronize the callback with the main
    # thread, as drawing should happen in the main thread
    q = queue.Queue()
    l = MyListener(q)
    cam.registerDataListenerExtended(l)
    cam.startCapture()
    # create a loop that will run for a time (default 15 seconds)
    process_event_queue (q, l, options.seconds)
    cam.stopCapture()

def process_event_queue (q, painter, seconds):
    # create a loop that will run for the given amount of time
    t_end = time.time() + seconds
    while time.time() < t_end:
        try:
            # try to retrieve an item from the queue.
            # this will block until an item can be retrieved
            # or the timeout of 1 second is hit
            item = q.get(True, 1)
        except queue.Empty:
            # this will be thrown when the timeout is hit
            break
        else:
            painter.paint (item)

if (__name__ == "__main__"):
    main()
