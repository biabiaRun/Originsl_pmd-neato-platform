#!/usr/bin/python3

import uuid

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase

class ExampleDevice:
    """A Zwetschge-using device, called "example device", and using separate files for the register
    maps. This device uses only TimedRegisterMaps, so can be used as a file on the host's
    filesystem, in either Zwetschge or Lena format.

    The directory containing this file must also have a file called "__init__.py" (the file can be
    empty).  This means that the directory is an implicit Python package, and other files in the
    same directory can be imported with relative imports, as is used for the mode_9_50fps.py file.

    It's assumed that there will be at least one large file containing register settings, and
    probably one of these large files for each use case.  The ZwetschgeTool makes this easier by
    importing this ExampleDevice.py file with the containing directory as its package.
    """

    from .mode_9_50fps import full_cfg as mode_9_50fps_registers

    # A 50 FPS use case is usually only 5-phase, but it's probably useful that the example won't be
    # mistaken for a real device's settings.
    uc_mode_9_50fps = UseCase ("example 9 phase",
        uuid.uuid5 (uuid.NAMESPACE_URL, "http://www.example.org/9phase"),
        timedRegList = mode_9_50fps_registers,
        imageSize = [172, 120],
        imagerFrequencies = [int (3e7), int (8e7), int (8e7), int (8e7), int (8e7), int (8e7), int (8e7), int (8e7), int (8e7)],
        streamIds = [0x1234],
        startFps = 50,
        fpsLimits = [1, 50], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuid.uuid5 (uuid.NAMESPACE_URL, "http://www.pmdtec.com/"),
        waitTime = 10,
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [9], # single superframe in a 9-raw-frame measurement block
        exposureGroups = [
            [200, [1, 800]],
            [250, [1, 1000]],
            [250, [1, 1000]],
            ],
        rawFrameSets = [
            [1, int (3e7), 0],
            [4, int (8e7), 1],
            [4, int (8e7), 2],
            ]
        )

    # The TableOfRegisterMaps given here is just filled with junk data, but the size of each map
    # matches the test data for the C++ unit tests (which themselves match C2_M2452_B1x.lena).
    #
    # The full register maps can also be put in separate file(s), as with the use cases' registers.
    torm = TableOfRegisterMaps (
        init = [[1, 11, 0]] * 2041,
        fwPage1 = [],
        fwPage2 = [],
        fwStart = [],
        start = [[2, 22, 0]] * 3,
        stop = [[3, 33, 0]] * 1,
        )

    device_spec = DeviceData (
        name = "example device",
        productIssuer = "TEST",
        productCode = uuid.uuid5 (uuid.NAMESPACE_URL, "http://www.example.com/"),
        systemFrequency = 26 * 1000 * 1000,
        useCases = [uc_mode_9_50fps],
        torm = torm
        )

def getZwetschgeDeviceData():
    """Entry point from ZwetschgeTool, when this file is used as the input file (with --device)"""
    device_spec = ExampleDevice.device_spec
    return device_spec
