#!/usr/bin/python3

# Copyright (C) 2018 Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""Data types that can be represented in the Zwetschge format."""

from enum import IntEnum

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
    def __init__ (self, init, fwPage1, fwPage2, fwStart, start, stop):
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

    def __init__ (self, name, guid, *, imageSize, imagerFrequencies, streamIds, startFps, fpsLimits, processingParams, waitTime, accessLevel, measurementBlocks, exposureGroups, rawFrameSets, timedRegList=None, seqRegBlock=None):
        self.name = name
        self.timedRegList = timedRegList
        self.seqRegBlock = seqRegBlock
        self.guid = guid
        self.imageSize = imageSize
        self.imagerFrequencies = imagerFrequencies
        self.streamIds = streamIds
        self.startFps = startFps
        self.fpsLimits = fpsLimits
        self.processingParams = processingParams
        self.waitTime = waitTime
        self.accessLevel = accessLevel
        self.measurementBlocks = measurementBlocks
        self.exposureGroups = exposureGroups
        self.rawFrameSets = rawFrameSets
        self.reservedBlock = bytes () #empty

    def __str__ (self):
        return 'UseCase (name="{name}", guid={guid}, timedRegList={timedRegList}, seqRegBlock={seqRegBlock})'.format (
            name=self.name, guid=self.guid, timedRegList=_lenOrNone (self.timedRegList), seqRegBlock=self.seqRegBlock)

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
        s = 'DeviceData (name="{name}" torm={torm}")'.format (name=self.name, torm=self.torm)
        for uc in self.useCases:
            s += "\n    " + str (uc)
        return s
