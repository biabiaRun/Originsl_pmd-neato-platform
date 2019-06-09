#!/usr/bin/python3

# Copyright (C) 2018 Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""Provides sanity checks on data for a Zwetschge device"""

from .data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase

from itertools import combinations

class Validator:
    def __init__ (self):
        pass

    def checkUseCase (self, uc):
        if uc.timedRegList is None and uc.seqRegBlock is None:
            raise ValueError ('Use Case "{name}" has neither a timedRegList nor a seqRegBlock'.format (name=uc.name))
        if uc.timedRegList is not None and uc.seqRegBlock is not None:
            raise ValueError ('Use Case "{name}" has both a timedRegList and a seqRegBlock'.format (name=uc.name))
        if uc.fpsLimits[0] > uc.startFps or uc.startFps > uc.fpsLimits[1]:
            raise ValueError ('Use Case "{name}" has startFps outside its FPS limits'.format (name=uc.name))
        for x in uc.exposureGroups:
            if x[1][0] > x[0] or x[0] > x[1][1]:
                raise ValueError ('Use Case "{name}" has a starting exposure outside that group\'s limits'.format (name=uc.name))
        for x in uc.rawFrameSets:
            if x[2] >= len (uc.exposureGroups):
                raise ValueError ('Use Case "{name}" has a rawFrameSet with an out-of-bounds exposureGroupIndex'.format (name=uc.name))
        freqs = []
        for x in uc.rawFrameSets:
            for num in range(0, x[0]):
                freqs.append(x[1])
        if len (freqs) != len (uc.imagerFrequencies):
            raise ValueError ('Use Case "{name}" has mismatched raw frame count vs imagerFrequencies'.format (name=uc.name))
        for idx in range(0, len (freqs)):
            if freqs[idx] != uc.imagerFrequencies[idx]:
                raise ValueError ('Use Case "{name}" has mismatched raw frame sets vs imagerFrequencies'.format (name=uc.name))
        # there are many numbers that match the rawFrameCount, choosing uc.measurementBlocks as the
        # "most likely to be correct", as it's often a single number
        rawFrameCount = sum (uc.measurementBlocks)
        if rawFrameCount != len (uc.imagerFrequencies):
            raise ValueError ('Use Case "{name}" has mismatched raw frame count vs imagerFrequencies'.format (name=uc.name))
        if rawFrameCount != sum ([x[0] for x in uc.rawFrameSets]):
            raise ValueError ('Use Case "{name}" has mismatched measurementBlocks vs rawFrameSets'.format (name=uc.name))

    def checkUseCasePair (self, a, b):
        if a.name == b.name:
            raise ValueError ('Multiple use cases have name "{name}"'.
                    format(name=a.name))
        if a.guid == b.guid:
            raise ValueError ('Use cases "{a}" and "{b}" have the same guid'.
                    format(a=a.name, b=b.name))
        # The following are to catch errors where an entry in the device.py is copy-and-pasted from
        # another entry, but the name of the corresponding register settings wasn't updated. They
        # will also prevent the valid case of deliberately having two use cases that only differ in
        # the processing parameters; edit this script if you intended to do that.
        if a.timedRegList is not None:
            if a.timedRegList == b.timedRegList:
                raise ValueError ('Use cases "{a}" and "{b}" have matching settings in the timedRegList'.
                        format(a=a.name, b=b.name))
        if a.seqRegBlock is not None:
            if a.seqRegBlock == b.seqRegBlock:
                raise ValueError ('Use cases "{a}" and "{b}" have matching settings in the seqRegBlock'.
                        format(a=a.name, b=b.name))

    def checkUseCases (self, useCases):
        for a,b in combinations (range (len (useCases)), 2):
            self.checkUseCasePair (useCases[a], useCases[b])
        for uc in useCases:
            self.checkUseCase (uc)

    def checkCalibration (self, calibration):
        pass

    def check (self, deviceData, calibration):
        self.checkUseCases (deviceData.useCases)
        self.checkCalibration (calibration)
