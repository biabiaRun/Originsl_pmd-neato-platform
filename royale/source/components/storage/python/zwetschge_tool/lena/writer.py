#!/usr/bin/python3

# Copyright (C) 2018 Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""Converting the ZwetschgeDataTypes to a Lena file.

Zwetschge is meant to replace Lena, but being able to generate Lena files from the same inputs is
likely to hasten the adoption of the ZwetschgeTool.

This assumes that the device is using TimedRegisterLists, not SequentialRegisterBlocks.
"""

from math import ceil
from struct import Struct
from zlib import crc32

def packSemicolonDelims (x):
    return ";".join (map (str, x))

def packTimedRegListEntry (x):
    """Conversion from either (address, value) or (address, value, microseconds)"""
    if len (x) == 3:
        return "{0:#06x};{1:#06x};{2}".format (x[0], x[1], x[2])
    elif len (x) == 2:
        return "{0:#06x};{1:#06x};0".format (x[0], x[1])
    raise ValueError ("Expected (address, value) or (address, value, delay)")

class LenaWriter:
    def __init__ (self, deviceData, calibration):
        """Construct from a ZwetschgeDataTypes.DeviceData object"""
        self._deviceData = deviceData
        if calibration is not None:
            raise ValueError ("Lena files don't support calibration data")

    def exportUseCase (self, uc):
        result = []

        result.append ("USECASE-START")
        result.append ("GUID;{" + str (uc.guid) + "}")
        result.append ("NAME;" + uc.name)
        result.append ("BLOCKS;" + packSemicolonDelims (uc.measurementBlocks))
        result.append ("MODFREQS;" + packSemicolonDelims (uc.imagerFrequencies))
        if uc.timedRegList is not None:
            for x in uc.timedRegList:
                result.append (packTimedRegListEntry (x))
        else:
            if uc.seqRegBlock is None:
                raise ValueError ("No registers given")
            curAddress = uc.seqRegBlock.imagerAddress
            for x in uc.seqRegBlock.values:
                result.append (packTimedRegListEntry ([curAddress, x]))
                curAddress = curAddress + 1
        result.append ("USECASE-END")

        return result

    def exportAsMultiline (self):
        """Export the data (returns a multi-line string representation)"""
        result = []
        result.append ("ROYALE-IMAGER-LENA-FILE")
        result.append ("VERSION;1003")

        result.append ("INIT-MAP")
        for x in self._deviceData.torm.init:
            result.append (packTimedRegListEntry (x))
        result.append ("INIT-MAP-END")
        result.append ("FW-PAGE-1")
        for x in self._deviceData.torm.fwPage1:
            result.append (packTimedRegListEntry (x))
        result.append ("FW-PAGE-1-END")
        result.append ("FW-PAGE-2")
        for x in self._deviceData.torm.fwPage2:
            result.append (packTimedRegListEntry (x))
        result.append ("FW-PAGE-2-END")
        result.append ("FW-START-MAP")
        for x in self._deviceData.torm.fwStart:
            result.append (packTimedRegListEntry (x))
        result.append ("FW-START-MAP-END")
        result.append ("START-MAP")
        for x in self._deviceData.torm.start:
            result.append (packTimedRegListEntry (x))
        result.append ("START-MAP-END")
        result.append ("STOP-MAP")
        for x in self._deviceData.torm.stop:
            result.append (packTimedRegListEntry (x))
        result.append ("STOP-MAP-END")

        for uc in self._deviceData.useCases:
            result.extend (self.exportUseCase (uc))
        return result

    def pack (self):
        """Export as a binary blob, containing the result of exportAsMultiline() in ASCII"""
        result = bytearray()
        for x in self.exportAsMultiline():
            result.extend (bytes (x + "\n", "ascii"))
        return result
