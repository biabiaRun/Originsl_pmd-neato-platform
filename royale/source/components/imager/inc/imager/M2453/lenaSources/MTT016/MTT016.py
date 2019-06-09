#!/usr/bin/python3

import uuid
from struct import Struct
from zlib import crc32
import sys

from zwetschge_tool.royale.common_processing_ids import *
from zwetschge_tool.util.M2453_B11_utils import genSeqRegisterBlock

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase

class MTT016:
    """The MTT016 module 


    The directory containing this file must also have a file called "__init__.py" (the file can be
    empty).  This means that the directory is an implicit Python package, and other files in the
    same directory can be imported with relative imports, as is used for the mode_9_1fps.py file.

    It's assumed that there will be at least one large file containing register settings, and
    probably one of these large files for each use case.  The ZwetschgeTool makes this easier by
    importing this MTT016.py file with the containing directory as its package.
    """

    # The TableOfRegisterMaps contains all maps except the per-use-case ones
    from .MTT016_reg_maps import torm

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Mode_9_5FPS_80_60MHz import full_cfg as mode_9_5_fps_timed_register_list
    
    use_case_9_5_fps = UseCase ("Low_Noise_Extended",
        uuid.UUID ("{373877FB-7649-4866-B1D2-0A97CF95B53C}"),
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
            [1990, [50, 1990]],
            [1990, [50, 1990]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Mode_9_30FPS_80_60MHz import full_cfg as mode_9_30_fps_timed_register_list

    use_case_9_30_fps = UseCase ("Video_Extended",
        uuid.UUID ("{577B8F92-C7A8-48F1-92FC-F118ABE3F80A}"),
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
            [330, [50, 330]],
            [330, [50, 330]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Mode_5_15FPS_60MHz import full_cfg as mode_5_15_fps_timed_register_list
    
    use_case_5_15_fps = UseCase ("Video_Half",
        uuid.UUID ("{D2026B53-7C41-4700-9F64-79B67B572511}"),
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
            [1340, [50, 1340]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Mode_5_30FPS_60MHz import full_cfg as mode_5_30_fps_timed_register_list
    
    use_case_5_30_fps = UseCase ("Video",
        uuid.UUID ("{32DE1EF3-CB22-4DD5-8534-CFDD2894AD06}"),
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
            [670, [50, 670]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Mode_5_45FPS_60MHz import full_cfg as mode_5_45_fps_timed_register_list
    
    use_case_5_45_fps = UseCase ("Fast_Acquisition",
        uuid.UUID ("{25DA7C8B-69D4-4C2A-82CE-DC5B238950C9}"),
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
            [450, [50, 450]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Mode_5_60FPS_60MHz import full_cfg as mode_5_60_fps_timed_register_list
    
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
            [330, [50, 330]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )
        
    device_spec = DeviceData (
        name = "MTT016",
        productIssuer = "PMD ",
        productCode = uuid.UUID ("{03000000000000000000000000000030}"),
        systemFrequency = 24 * 1000 * 1000,
        useCases = [
            use_case_9_5_fps,
            use_case_9_30_fps,
            use_case_5_15_fps,
            use_case_5_30_fps,
            use_case_5_45_fps,
            use_case_5_60_fps,
            ],
        torm = torm
        )

def getZwetschgeDeviceData():
    """Entry point from ZwetschgeTool, when this file is used the input file (with --device)"""
    
    device_spec = MTT016.device_spec
    return device_spec
