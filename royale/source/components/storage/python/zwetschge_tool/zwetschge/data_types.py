#!/usr/bin/python3

# Copyright (C) 2018 Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""Data types that can be represented in the Zwetschge format."""

from enum import IntEnum

FLASH_OFFSET = 0x2000

def _lenOrNone (x):
    """Helper fuction for printing lists"""
    if x is None:
        return "None"
    return "(list with len=={lenX})".format (lenX=len (x))


class SequentialRegisterBlock:
    """A block of register values that the imager should read directly from the storage. The
    address in the flash will be chosen by the ZwetschgeWriter.

    In Royale, these are accessed via the C++ SequentialRegisterHeader.
    """
    def __init__ (self, values, imagerAddress):
        self.values = values
        self.imagerAddress = imagerAddress

    def __str__ (self):
        return ("(values=%s, imagerAddress=%x)" %
            (_lenOrNone (self.values), self.imagerAddress))

class TableOfRegisterMaps:
    """Contains the set of TimedRegisterMaps which are per-device and not per-use-case.

    SequentialRegisterBlocks are currently not supported.
    """
    def __init__ (self, init=None, fwPage1=None, fwPage2=None, fwStart=None, start=None, stop=None):
        self.init = init
        self.fwPage1 = fwPage1
        self.fwPage2 = fwPage2
        self.fwStart = fwStart
        self.start = start
        self.stop = stop

    def __str__ (self):
        return 'ToRM (lengths={{{init}, {fwPage1}, {fwPage2}, {fwStart}, {start}, {stop}}})'.format (
            init=len (self.init), fwPage1=len (self.fwPage1), fwPage2=len (self.fwPage2),
            fwStart=len (self.fwStart), start=len (self.start), stop=len (self.stop))

class AccessLevel (IntEnum):
    NORMAL = 0
    LEVEL_THREE_RAW = 1

class UseCase:
    """For documentation refer to the Zwetschge specification. Comments here only cover the details
    that are specific to this implementation.

    The TimedRegisterLists can contain a mixture of (address, value) and (address, value,
    microseconds) tuples. For 2-item tuples, the delay is assumed to be zero.
    """

    def __init__ (self, name=None, guid=None, *, imageSize=None, imagerFrequencies=None, streamIds=None, startFps=None,
                  fpsLimits=None, processingParams=None, waitTime=None, accessLevel=None, measurementBlocks=None,
                  exposureGroups=None, rawFrameSets=None, timedRegList=None, seqRegBlock=None, reservedBlock=bytes(),
                  seqRegBlockAddress=None, seqRegBlockLen=None, seqRegBlockImagerAddress=None):
        self.name = name
        self.timedRegList = timedRegList
        self.seqRegBlock = seqRegBlock
        self.guid = guid
        if imageSize is None:
            imageSize = []
        self.imageSize = imageSize
        if imagerFrequencies is None:
            imagerFrequencies = []
        self.imagerFrequencies = imagerFrequencies
        if streamIds is None:
            streamIds = []
        self.streamIds = streamIds
        self.startFps = startFps
        if fpsLimits is None:
            fpsLimits = []
        self.fpsLimits = fpsLimits
        self.processingParams = processingParams
        self.waitTime = waitTime
        self.accessLevel = accessLevel
        if measurementBlocks is None:
            measurementBlocks = []
        self.measurementBlocks = measurementBlocks
        if exposureGroups is None:
            exposureGroups = []
        self.exposureGroups = exposureGroups
        if rawFrameSets is None:
            rawFrameSets = []
        self.rawFrameSets = rawFrameSets
        self.reservedBlock = reservedBlock
        self.seqRegBlockAddress = seqRegBlockAddress
        self.seqRegBlockLen = seqRegBlockLen
        self.seqRegBlockImagerAddress = seqRegBlockImagerAddress

    def __str__ (self):
        streamlist = [str(hex(x)) for x in self.streamIds]
        return 'UseCase ( \n \
                         name="{name}", \n \
                         guid={guid}, \n \
                         timedRegList={timedRegList}, \n \
                         seqRegBlock={seqRegBlock}, \n \
                         imageSize={imageSize}, \n \
                         imagerFrequencies={imagerFrequencies}, \n \
                         streamIds={streamIds}, \n \
                         startFps={startFps}, \n \
                         fpsLimits={fpsLimits}, \n \
                         processingParams={processingParams}, \n \
                         waitTime={waitTime}, \n \
                         accessLevel={accessLevel}, \n \
                         measurementBlocks={measurementBlocks}, \n \
                         exposureGroups={exposureGroups}, \n \
                         rawFrameSets={rawFrameSets} \n \
                         )'.format (
            name=self.name, guid=self.guid, timedRegList=_lenOrNone (self.timedRegList), seqRegBlock=self.seqRegBlock,
            imageSize=self.imageSize, imagerFrequencies=self.imagerFrequencies, streamIds=streamlist,
            startFps=self.startFps, fpsLimits=self.fpsLimits, processingParams=self.processingParams.bytes, waitTime=self.waitTime,
            accessLevel=str(AccessLevel(self.accessLevel)), measurementBlocks=self.measurementBlocks, exposureGroups=self.exposureGroups,
            rawFrameSets=self.rawFrameSets)


    def get_config(self, zwetschge_data):
        """
        Get the register settings stored in the Zwetschge Data for the Use Case

        :param zwetschge_data: Binary data of full Zwetschge File
        :return:
        """
        self.seqRegBlock = zwetschge_data[self.seqRegBlockAddress - FLASH_OFFSET:self.seqRegBlockAddress + self.seqRegBlockLen - FLASH_OFFSET]

class DeviceData:
    def __init__ (self, name, *, productIssuer, productCode, systemFrequency=0, useCases=None, torm=None):
        self.name = name
        self.productIssuer = productIssuer
        self.productCode = productCode
        self.systemFrequency = systemFrequency
        if useCases is not None:
            self.useCases = useCases
        else:
            self.useCases = []
        if torm is not None:
            self.torm = torm
        else:
            self.torm = TableOfRegisterMaps ([], [], [], [], [], [])

    def addUseCase (self, usecase):
        """Append an additional use case to the ones already passed to __init__().

        Note that the constructor may make a shallow copy, so this method may alter the object
        passed to the constructor.

        Implementation defined behavior for "may", read "may (and currently does)".
        """
        self.useCases.append (usecase)

    def __str__ (self):
        s = 'DeviceData ( \n \
                         name="{name}" \n \
                         torm={torm}" \n \
                         productIssuer={productIssuer}, \n \
                         productCode={productCode}, \n \
                         systemFrequency={systemFrequency}, \n \
                         )'.format (name=self.name, torm=self.torm,
                         productIssuer=self.productIssuer, productCode=self.productCode, 
                         systemFrequency=self.systemFrequency)
        for uc in self.useCases:
            s += "\n    " + str (uc)
        return s
