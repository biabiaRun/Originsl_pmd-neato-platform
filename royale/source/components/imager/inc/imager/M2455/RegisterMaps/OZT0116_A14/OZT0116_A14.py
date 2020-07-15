#!/usr/bin/python3

import os
import uuid
from struct import Struct
from zlib import crc32
import sys

from zwetschge_tool.royale.common_processing_ids import *
from zwetschge_tool.util.M2453_B11_utils import genSeqRegisterBlock
from zwetschge_tool.util.M2453_B11_utils import getInitMap

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase
from zwetschge_tool.util.import_py_regmap import createUseCase

class OZT0116_A14:
    """The OZT0116_A14 module 


    The directory containing this file must also have a file called "__init__.py" (the file can be
    empty).  This means that the directory is an implicit Python package, and other files in the
    same directory can be imported with relative imports, as is used for the mode_9_1fps.py file.

    It's assumed that there will be at least one large file containing register settings, and
    probably one of these large files for each use case.  The ZwetschgeTool makes this easier by
    importing this OZT0116_A14.py file with the containing directory as its package.
    """

    registerMapPath = os.path.dirname(os.path.abspath(__file__))
    imagesize = [448, 168]
    
    use_case_low_noise_extended_60_80 = createUseCase(
            "Low_Noise_Extended_60_80", 
            "MCLK_24MHz_MID_0043_Low_Noise_Extended_60_80.py",
            registerMapPath,            
            imagesize,
            uuidLowNoiseExtendedParam, 
            AccessLevel.NORMAL)

    use_case_video_extended_60_80 = createUseCase(
            "Video_Extended_60_80", 
            "MCLK_24MHz_MID_0043_Video_Extended_60_80.py",
            registerMapPath,            
            imagesize,
            uuidVideoExtendedParam, 
            AccessLevel.NORMAL)
            
    use_case_video_half_80 = createUseCase(
            "Video_Half_80", 
            "MCLK_24MHz_MID_0043_Video_Half_80.py",
            registerMapPath,            
            imagesize,
            uuidVideoHalfParam, 
            AccessLevel.NORMAL)

    use_case_video_80 = createUseCase(
            "Video_80", 
            "MCLK_24MHz_MID_0043_Video_80.py",
            registerMapPath,            
            imagesize,
            uuidVideoParam, 
            AccessLevel.NORMAL)

    init = getInitMap(["MCLK_24MHz_MID_0043_Low_Noise_Extended_60_80.py",
                       "MCLK_24MHz_MID_0043_Video_Extended_60_80.py",
                       "MCLK_24MHz_MID_0043_Video_Half_80.py",
                       "MCLK_24MHz_MID_0043_Video_80.py"],
                       registerMapPath)

    fwPage1 = []

    fwPage2 = []

    fwStart = [
        (0xA02A, 0x0001, 0),
    ]

    start = [
        (0x9400, 0x0001, 0),
    ]

    stop = [
        (0x9400, 0x0000, 500000),
    ]

    torm = TableOfRegisterMaps (
        init,
        fwPage1,
        fwPage2,
        fwStart,
        start,
        stop
        )
            
            
    device_spec = DeviceData (
        name = "OZT0116_A14",
        productIssuer = "PMD ",
        productCode = uuid.UUID ("{03000000000000000000000000000043}"),
        systemFrequency = 24 * 1000 * 1000,
        useCases = [
            use_case_low_noise_extended_60_80,
            use_case_video_extended_60_80,
            use_case_video_half_80,
            use_case_video_80,
            ],
        torm = torm
        )

def getZwetschgeDeviceData():
    """Entry point from ZwetschgeTool, when this file is used the input file (with --device)"""
    
    device_spec = OZT0116_A14.device_spec
    return device_spec
