#!/usr/bin/python3

import os
import uuid
from struct import Struct
from zlib import crc32
import sys

from zwetschge_tool.royale.common_processing_ids import *
from zwetschge_tool.util.M2453_B11_utils import genSeqRegisterBlock

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase
from zwetschge_tool.util.import_py_regmap import createUseCase

class OZT0114:
    """The OZT0114 module 


    The directory containing this file must also have a file called "__init__.py" (the file can be
    empty).  This means that the directory is an implicit Python package, and other files in the
    same directory can be imported with relative imports, as is used for the mode_9_1fps.py file.

    It's assumed that there will be at least one large file containing register settings, and
    probably one of these large files for each use case.  The ZwetschgeTool makes this easier by
    importing this OZT0114.py file with the containing directory as its package.
    """

    # The TableOfRegisterMaps contains all maps except the per-use-case ones
    from .OZT0114_reg_maps import torm
    
    registerMapPath = os.path.dirname(os.path.abspath(__file__))
    imagesize = [224, 172]

    use_case_low_noise_extended = createUseCase(
            "Low_Noise_Extended", 
            "MCLK_24MHz_MID_0041_Low_Noise_Extended_60_80.py",
            registerMapPath,            
            imagesize,
            uuidLowNoiseExtendedParam, 
            AccessLevel.NORMAL)
    
    use_case_video_extended = createUseCase(
            "Video_Extended", 
            "MCLK_24MHz_MID_0041_Video_Extended_60_80.py", 
            registerMapPath,            
            imagesize,
            uuidVideoExtendedParam, 
            AccessLevel.NORMAL)

    use_case_video_half = createUseCase(
            "Video_Half", 
            "MCLK_24MHz_MID_0041_Video_Half_80.py", 
            registerMapPath,            
            imagesize,
            uuidVideoHalfParam, 
            AccessLevel.NORMAL)

    use_case_video = createUseCase(
            "Video", 
            "MCLK_24MHz_MID_0041_Video_80.py", 
            registerMapPath,            
            imagesize,
            uuidVideoParam, 
            AccessLevel.NORMAL)
                                            
    device_spec = DeviceData (
        name = "OZT0114",
        productIssuer = "PMD ",
        productCode = uuid.UUID ("{03000000000000000000000000000041}"),
        systemFrequency = 24 * 1000 * 1000,
        useCases = [
            use_case_low_noise_extended,
            use_case_video_extended,
            use_case_video_half,
            use_case_video,
            ],
        torm = torm
        )

def getZwetschgeDeviceData():
    """Entry point from ZwetschgeTool, when this file is used the input file (with --device)"""
    
    device_spec = OZT0114.device_spec
    return device_spec
