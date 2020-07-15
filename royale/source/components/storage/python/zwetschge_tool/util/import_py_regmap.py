import importlib
import importlib.util
import sys
import os
import re
import logging

import uuid
from struct import Struct
from zlib import crc32

from zwetschge_tool.royale.common_processing_ids import *
from zwetschge_tool.royale.uuidlike_identifier import hashUuidlikeIdentifier
from zwetschge_tool.util.M2453_B11_utils import genSeqRegisterBlock

from zwetschge_tool.zwetschge.data_types import AccessLevel, DeviceData, TableOfRegisterMaps, UseCase

logger = logging.getLogger('pmdpy')

rx_dict = {
    'rawframes': re.compile(r'RawFrames: (\d+)\n'),
    'framerate': re.compile(r'Frame Rate: (\d+) fps\n'),
    'tintMax': re.compile(r'Tint_max: (.+)'),
    'tintStart': re.compile(r'Tint_start: (.+)'),
    'minIdle': re.compile(r'.+min. Idle Time = (.+) s'),
}

rx_dict2 = {
    'tints': re.compile(r'(\d+[,.]?\d*)ms @ (\d+[,.]?\d*)MHz \((Goff|P4|Gon)\)'),
}

def _getExpoFreqMode(match):
    groups = match.groups()
    if len(groups) != 3:
        raise Exception ("_getExpoFreqMode : Wrong number of groups")
    expotime = int(float(groups[0].replace(',', '.')) * 1e3)
    freq = int(float(groups[1].replace(',', '.')) * 1e6)
    if groups[2] == "Goff":
        numframes = 1
    elif groups[2] == "Gon":
        numframes = 1
    elif groups[2] == "P4":
        numframes = 4
    else:
        raise Exception ("_getExpoFreqMode : Wrong mode")
    return expotime, freq, numframes

def _parse_line(line, redict):
    for key, rx in redict.items():
        match = rx.search(line)
        if match:
            return key, match
    # if there are no matches
    return None, None

def parsePyFile (sourceFileAbsolute):
    file = open(sourceFileAbsolute, "r")

    tintMax = []
    tintStart = []
    freq1 = []
    freq2 = []
    rfs1 = []
    rfs2 = []
    framerate = 0
    numFrames = 0
    idleTime = 1e6

    lines = file.readlines()
    for line in lines:
        key, match = _parse_line(line, rx_dict)
        if key == 'tintMax':
            newlines = line.split('/')
            for newline in newlines:
                key, match = _parse_line(newline, rx_dict2)
                if key != None:
                    expo, freq, numframes = _getExpoFreqMode(match)
                    tintMax.append(expo)
                    freq1.append(freq)
                    rfs1.append(numframes)

        if key == 'tintStart':
            newlines = line.split('/')
            for newline in newlines:
                key, match = _parse_line(newline, rx_dict2)
                if key != None:
                    expo, freq, numframes = _getExpoFreqMode(match)
                    tintStart.append(expo)
                    freq2.append(freq)
                    rfs2.append(numframes)

        if key == 'framerate':
            framerate = int(match.group(1))

        if key == 'rawframes':
            numFrames = int(match.group(1))

        if key == 'minIdle':
            idleTime = int(1e6 * float(match.group(1).replace(',', '.')))

    if freq1 != freq2:
        raise Exception ("parsePyFile : Frequencies don't match in " + sourceFileAbsolute)

    if rfs1 != rfs2:
        raise Exception ("parsePyFile : Raw frame sets don't match in " + sourceFileAbsolute)

    if len(freq1) != len(rfs1):
        raise Exception ("parsePyFile : Lengths don't match in " + sourceFileAbsolute)

    if framerate == 0:
        raise Exception ("parsePyFile : framerate shouldn't be zero in " + sourceFileAbsolute)

    if len(freq1) == 0:
        raise Exception ("parsePyFile : could not find any frequencies in " + sourceFileAbsolute)

    if len(tintMax) == 0:
        raise Exception ("parsePyFile : could not find any tintMax in " + sourceFileAbsolute)

    if len(tintStart) == 0:
        raise Exception ("parsePyFile : could not find any tintStart in " + sourceFileAbsolute)

    if len(rfs1) == 0:
        raise Exception ("parsePyFile : could not find any frame group sizes in " + sourceFileAbsolute)

    return tintMax, tintStart, freq1, rfs1, framerate, idleTime


def importPyRegisterMap (sourceFile):
    name = sourceFile.strip('.py')
    name = "." + name
    module = importlib.import_module(name, "device_package")

    return module.full_cfg


def createUseCase (_useCaseName, _sourceFilename, _registerMapPath, _imageSize, _procParams, _accessLevel):

    sourceFileAbsolute = os.path.join(_registerMapPath, _sourceFilename)
    tintMax, tintStart, freqs, rfs, framerate, idleTime = parsePyFile(sourceFileAbsolute)

    _uuid = hashUuidlikeIdentifier(_useCaseName)
    _seqRegBlock = genSeqRegisterBlock(importPyRegisterMap(_sourceFilename))
    _streamIds = [0x1234] # This only supports one stream
    _startFps = framerate
    _fpsLimits = [1, framerate]
    _waitTime = idleTime

    _imagerFrequencies = []
    _measurementBlocks = 0
    _exposureGroups = []
    _rawFrameSets = []
    for i in range(0, len(rfs)):
        _measurementBlocks += rfs[i]
        _rawFrameSets.append([rfs[i], freqs[i], i])
        tintMin = 0
        if rfs[i] == 1:
            tintMin = tintMax[i]
        else:
            tintMin = 50
        _exposureGroups.append([tintStart[i], [tintMin, tintMax[i]]])
        for j in range(0, rfs[i]):
            _imagerFrequencies.append(freqs[i])
    _measurementBlocks = [_measurementBlocks]

    a = UseCase (_useCaseName,
        _uuid,
        seqRegBlock = _seqRegBlock,
        imageSize = _imageSize,
        imagerFrequencies = _imagerFrequencies,
        streamIds = _streamIds,
        startFps = _startFps,
        fpsLimits = _fpsLimits,
        processingParams = _procParams,
        waitTime = _waitTime,
        accessLevel = _accessLevel,
        measurementBlocks = _measurementBlocks,
        exposureGroups = _exposureGroups,
        rawFrameSets = _rawFrameSets
        )
    return a
