#!/usr/bin/python3

import uuid
from struct import Struct
from zlib import crc32
import sys

from zwetschge_tool.royale.common_processing_ids import *
from zwetschge_tool.util.M2453_B11_utils import genSeqRegisterBlock

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase

class Orpheus:
    """The Orpheus module 


    The directory containing this file must also have a file called "__init__.py" (the file can be
    empty).  This means that the directory is an implicit Python package, and other files in the
    same directory can be imported with relative imports, as is used for the mode_9_1fps.py file.

    It's assumed that there will be at least one large file containing register settings, and
    probably one of these large files for each use case.  The ZwetschgeTool makes this easier by
    importing this Orpheus.py file with the containing directory as its package.
    """

    # The TableOfRegisterMaps contains all maps except the per-use-case ones
    from .Orpheus_reg_maps import torm

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Low_Noise_Extended import full_cfg as low_noise_extended_register_list
    
    use_case_low_noise_extended = UseCase ("Low_Noise_Extended",
        uuid.UUID ("{D143BDF1-356E-4307-9165-E7787238D9A2}"),
        seqRegBlock = genSeqRegisterBlock(low_noise_extended_register_list),
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
            [100, [100, 100]],
            [1300, [50, 1300]],
            [1300, [50, 1300]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Video_Extended import full_cfg as video_extended_register_list

    use_case_video_extended = UseCase ("Video_Extended",
        uuid.UUID ("{682A4DBA-A734-493A-A0CA-7BF952B00857}"),
        seqRegBlock = genSeqRegisterBlock(video_extended_register_list),
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
            [100, [100, 100]],
            [220, [50, 220]],
            [220, [50, 220]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (80320000), 1],
            [4, int (60240000), 2],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Video_Half import full_cfg as video_half_register_list
    
    use_case_video_half = UseCase ("Video_Half",
        uuid.UUID ("{C564D960-4531-4294-B5C6-AD9C141AD46F}"),
        seqRegBlock = genSeqRegisterBlock(video_half_register_list),
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
            [100, [100, 100]],
            [910, [50, 910]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Video import full_cfg as video_register_list
    
    use_case_video = UseCase ("Video",
        uuid.UUID ("{B5E47CAB-F26A-41B8-BC1B-31C2ACC0FABE}"),
        seqRegBlock = genSeqRegisterBlock(video_register_list),
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
            [100, [100, 100]],
            [450, [50, 450]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Fast_Acquisition import full_cfg as fast_acquisition_register_list
    
    use_case_fast_acquisition = UseCase ("Fast_Acquisition",
        uuid.UUID ("{86FF590F-DDB6-408B-8258-D58B41125F17}"),
        seqRegBlock = genSeqRegisterBlock(fast_acquisition_register_list),
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
            [100, [100, 100]],
            [300, [50, 300]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )

    # This file contains the registers for a single use case, in `full_cfg`
    from .MCLK_24MHz_Very_Fast_Acquisition import full_cfg as very_fast_register_list
    
    use_case_very_fast = UseCase ("Very_Fast_Acquisition",
        uuid.UUID ("{D3718B3A-FFF5-4091-8D55-6F2413D85A81}"),
        seqRegBlock = genSeqRegisterBlock(very_fast_register_list),
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
            [100, [100, 100]],
            [220, [50, 220]],
            ],
        rawFrameSets = [
            [1, int (60240000), 0],
            [4, int (60240000), 1],
            ]
        )
        
    from .MCLK_24MHz_Calibration import full_cfg as calib_register_list
    use_case_calib = UseCase ("Calibration",
        uuid.UUID ("{7D2033B2-05EA-433F-811A-66D20B6581FB}"),
        seqRegBlock = genSeqRegisterBlock(calib_register_list),
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
            [100, [100, 100]],
            [1030, [50, 1030]],
            [1030, [50, 1030]],
            [1500, [50, 1500]],
            [1500, [50, 1500]],
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
        name = "Orpheus",
        productIssuer = "PMD ",
        productCode = uuid.UUID ("{03000000000000000000000000000009}"),
        systemFrequency = 24 * 1000 * 1000,
        useCases = [
            use_case_low_noise_extended,
            use_case_video_extended,
            use_case_video_half,
            use_case_video,
            use_case_fast_acquisition,
            use_case_very_fast,
            use_case_calib,
            ],
        torm = torm
        )

def getZwetschgeDeviceData():
    """Entry point from ZwetschgeTool, when this file is used the input file (with --device)"""
    
    device_spec = Orpheus.device_spec
    return device_spec
