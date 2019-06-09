#!/usr/bin/python3

import uuid
from struct import Struct
from zlib import crc32
import sys

from zwetschge_tool.royale.common_processing_ids import *
from zwetschge_tool.util.M2453_B11_utils import genSeqRegisterBlock

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase

class Orat45:
    """The Orat45 module 


    The directory containing this file must also have a file called "__init__.py" (the file can be
    empty).  This means that the directory is an implicit Python package, and other files in the
    same directory can be imported with relative imports, as is used for the mode_9_1fps.py file.

    It's assumed that there will be at least one large file containing register settings, and
    probably one of these large files for each use case.  The ZwetschgeTool makes this easier by
    importing this Orat45.py file with the containing directory as its package.
    """

    # The TableOfRegisterMaps contains all maps except the per-use-case ones
    from .Orat45_reg_maps import torm

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_MODE_9_5FPS import full_cfg as mode_9_5_fps_timed_register_list
    
    use_case_9_5_fps = UseCase ("Low_Noise_Extended",
        uuid.UUID ("{D61E0988-6FB9-43D1-9AE6-11139F2DEAEF}"),
        seqRegBlock = genSeqRegisterBlock(mode_9_5_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 80320000, 80320000, 80320000, 80320000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 5,
        fpsLimits = [1, 5], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidLowNoiseExtendedParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [9], # single superframe in a 5-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [1590, [50, 1590]],
            [1590, [50, 1590]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_MODE_9_30FPS import full_cfg as mode_9_30_fps_timed_register_list

    use_case_9_30_fps = UseCase ("Video_Extended",
        uuid.UUID ("{0009601C-7D91-4FD3-B56F-D285283FCD0A}"),
        seqRegBlock = genSeqRegisterBlock(mode_9_30_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 80320000, 80320000, 80320000, 80320000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 30,
        fpsLimits = [1, 30], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidVideoExtendedParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [9], # single superframe in a 5-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [290, [50, 290]],
            [290, [50, 290]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_MODE_5_15FPS import full_cfg as mode_5_15_fps_timed_register_list
    
    use_case_5_15_fps = UseCase ("Video_Half",
        uuid.UUID ("{9FED6738-D002-4479-AB29-057A87089A69}"),
        seqRegBlock = genSeqRegisterBlock(mode_5_15_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 15,
        fpsLimits = [1, 15], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidVideoHalfParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [5], # single superframe in a 5-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [1100, [50, 1100]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_MODE_5_30FPS import full_cfg as mode_5_30_fps_timed_register_list
    
    use_case_5_30_fps = UseCase ("Video",
        uuid.UUID ("{5C89255E-2759-406B-8797-58A582D512EF}"),
        seqRegBlock = genSeqRegisterBlock(mode_5_30_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 30,
        fpsLimits = [1, 30], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidVideoParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [5], # single superframe in a 5-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [570, [50, 570]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_MODE_5_45FPS import full_cfg as mode_5_45_fps_timed_register_list
    
    use_case_5_45_fps = UseCase ("Fast_Acquisition",
        uuid.UUID ("{F0FBCD98-95B2-44CD-A541-B0391D3A4C34}"),
        seqRegBlock = genSeqRegisterBlock(mode_5_45_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 45,
        fpsLimits = [1, 45], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidFastAcquisitionParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [5], # single superframe in a 5-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [370, [50, 370]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_MODE_5_60FPS import full_cfg as mode_5_60_fps_timed_register_list
    
    use_case_5_60_fps = UseCase ("Very_Fast_Acquisition",
        uuid.UUID ("{28985A69-53CE-4049-8AA1-981E57F7CF41}"),
        seqRegBlock = genSeqRegisterBlock(mode_5_60_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 60,
        fpsLimits = [1, 60], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidVeryFastAcquisitionParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.NORMAL,
        measurementBlocks = [5], # single superframe in a 5-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [290, [50, 290]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )
        
    from .MCLK_24MHz_MODE_11_5FPS import full_cfg as mode_calib_5_fps_timed_register_list
    use_case_calib_5_fps = UseCase ("MODE_11_5_CALIBRATION",
        uuid.UUID ("{F8EC75B0-9647-4D86-9BD0-36BF9BAE6A3A}"),
        seqRegBlock = genSeqRegisterBlock(mode_calib_5_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 80320000, 80320000, 80320000, 80320000, 60240000, 60240000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 5,
        fpsLimits = [1, 5], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidLowNoiseExtendedParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.LEVEL_THREE_RAW,
        measurementBlocks = [11], # single superframe in a 11-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [1700, [50, 1700]],
            [1700, [50, 1700]],
            [1700, [50, 1700]],
            [1700, [50, 1700]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            [1, int (60240000), 3],
            [1, int (60240000), 4],
            ]
        )

    from .MCLK_24MHz_MODE_11_10FPS import full_cfg as mode_calib_10_fps_timed_register_list
    use_case_calib_10_fps = UseCase ("MODE_11_10_CALIBRATION",
        uuid.UUID ("{5E5C5733-0F9F-43C7-B646-E15D5E5A561D}"),
        seqRegBlock = genSeqRegisterBlock(mode_calib_10_fps_timed_register_list),
        imageSize = [224, 172],
        imagerFrequencies = [ 60240000, 80320000, 80320000, 80320000, 80320000, 60240000, 60240000, 60240000, 60240000, 60240000, 60240000 ],
        streamIds = [0x1234], # this will be overriden with the default stream id until ZwetschgeTool supports mixe mode
        startFps = 10,
        fpsLimits = [1, 10], # start FPS and limits
        # \todo ROYAL-3353 have standard uuids for our standard processing parameters
        processingParams = uuidLowNoiseExtendedParam,
        waitTime = int (1e6), # tail time not known, so using 1 second
        accessLevel = AccessLevel.LEVEL_THREE_RAW,
        measurementBlocks = [11], # single superframe in a 11-raw-frame measurement block
        exposureGroups = [
            [128, [128, 128]],
            [900, [50, 900]],
            [900, [50, 900]],
            [900, [50, 900]],
            [900, [50, 900]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            [1, int (60240000), 3],
            [1, int (60240000), 4],
            ]
        )
        
    device_spec = DeviceData (
        name = "Orat45",
        productIssuer = "PMD ",
        productCode = uuid.UUID ("{0300000000000000000000000000000E}"),
        systemFrequency = 24 * 1000 * 1000,
        useCases = [
            use_case_9_5_fps,
            use_case_9_30_fps,
            use_case_5_15_fps,
            use_case_5_30_fps,
            use_case_5_45_fps,
            use_case_5_60_fps,
            use_case_calib_5_fps,
            use_case_calib_10_fps,
            ],
        torm = torm
        )

def getZwetschgeDeviceData():
    """Entry point from ZwetschgeTool, when this file is used the input file (with --device)"""
    
    device_spec = Orat45.device_spec
    return device_spec
