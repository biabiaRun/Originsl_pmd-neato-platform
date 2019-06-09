#!/usr/bin/python3

# Copyright (C) 2018 Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

from math import ceil
from struct import Struct
from zlib import crc32

def pack24 (i):
    """Pack a uint24_t in to three bytes, little endian"""
    if i >= 2**24:
        raise InvalidValue ("too big for 24-bit storage")
    if i < 0:
        raise InvalidValue ("unsigned numbers only")
    return bytes ([i & 0xff, (i >> 8) & 0xff, (i >> 16) & 0xff])

def pack24p24s (p, s):
    """Pack one of Zwetschge's pointer + size pairs in to six bytes, little endian"""
    return pack24 (p) + pack24 (s)

def pack24p24s16a (p, s, a):
    """Pack one of Zwetschge's SequentialRegisterHeaders"""
    return pack24 (p) + pack24 (s) + Struct("<H").pack (a)

def packTimedRegListEntry (x):
    """Conversion from either (address, value) or (address, value, microseconds) to the 6-byte
    structure used in the TimedRegisterList, where the time unit is 32 microseconds.
    """
    if len (x) == 2:
        delay = 0
    elif len (x) == 3:
        delay = ceil (x[2] / 32)
    else:
        raise ValueError ("Expected (address, value) or (address, value, delay)")
    return Struct ("<H H H").pack (x[0], x[1], delay)

def roundUpToPage (x):
    """Page-align an offset, rounding upwards to the next multiple of 0x100"""
    m = x % 0x100
    if m == 0:
        return x
    return (x + 0x100 - m)

class Layout:
    """Internal bookkeeping for the offsets in to the image"""
    def __init__ (self):
        self.toc = 0
        self.torm = 0
        self.touc = 0
        # map, using UseCase.guid as the keys, and the corresponding offset as the values
        self.useCaseSeqRegBlocks = {}
        self.calibrationCrc = 0
        self.calibrationLen = 0
        self.calibrationStart = 0
        self.moduleSuffixLen = 0
        self.moduleSuffixStart = 0

class ZwetschgeWriter:
    """Converts the ZwetschgeDataTypes to binary data.

    The M2453 imagers include hardware support for accessing an SPI-connected storage device. They can
    automatically load firmware from the storage, and can load register maps directly from the storage.
    A new data layout, Zwetschge, stores the calibration and information about supported use cases,
    which Royale needs to read. It also contains the register settings for use cases, which can either
    be read directly by the imager, or read in to Royale and then written back to the imager.

    This is currently based on version 0x147 of the Zwetschge spec.
    """

    def __init__ (self, deviceData, calibration, moduleSerial, moduleSuffix):
        self._deviceData = deviceData
        self._calibration = calibration
        if not moduleSerial:
            self._moduleSerial = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        else:
            self._moduleSerial = moduleSerial
        if not moduleSuffix:
            self._moduleSuffix = None
        else:    
            self._moduleSuffix = bytes (moduleSuffix, 'ascii')

    def exportSeqRegBlock (self, seqRegBlock):
        result = bytearray ()
        struct = Struct (">H")
        for x in seqRegBlock.values:
            result.extend (struct.pack (x))
        return result

    def lenSeqRegBlock (self, seqRegBlock):
        return 2 * len (seqRegBlock.values)

    def exportUseCase (self, uc, seqRegAddress):
        result = bytearray ()

        # sequential register header, assume unused for now
        if uc.seqRegBlock is None:
            result.extend (pack24p24s16a (0, 0, 0))
        else:
            result.extend (pack24p24s16a (seqRegAddress, self.lenSeqRegBlock (uc.seqRegBlock), uc.seqRegBlock.imagerAddress))

        result.extend (Struct ("<H H").pack (uc.imageSize[0], uc.imageSize[1]))
        result.extend (uc.guid.bytes)
        result.extend (Struct ("B B B").pack (uc.startFps, uc.fpsLimits[0], uc.fpsLimits[1]))
        result.extend (uc.processingParams.bytes)
        result.extend (pack24 (uc.waitTime))
        result.extend (Struct ("B").pack (uc.accessLevel))

        result.extend (Struct ("B").pack (len (uc.name)))
        result.extend (Struct ("<H").pack (len (uc.measurementBlocks)))
        result.extend (Struct ("<H").pack (len (uc.imagerFrequencies)))
        if uc.timedRegList is None:
            result.extend (Struct ("<H").pack (0))
        else:
            result.extend (Struct ("<H").pack (len (uc.timedRegList)))
        result.extend (Struct ("B").pack (len (uc.streamIds)))
        result.extend (Struct ("B").pack (len (uc.exposureGroups)))
        result.extend (Struct ("<H").pack (len (uc.rawFrameSets)))
        result.extend (Struct ("B").pack (len (uc.reservedBlock)))

        result.extend (uc.name.encode ('ascii'))
        for x in uc.measurementBlocks:
            result.extend (Struct ("<H").pack (x))
        for x in uc.imagerFrequencies:
            result.extend (Struct ("<I").pack (x))
        if uc.timedRegList is not None:
            for x in uc.timedRegList:
                result.extend (packTimedRegListEntry (x))
        for x in uc.streamIds:
            result.extend (Struct ("<H").pack (x))
        for x in uc.exposureGroups:
            result.extend (Struct ("<H H H").pack (x[0], x[1][0], x[1][1]))
        for x in uc.rawFrameSets:
            result.extend (Struct ("<B I B").pack (x[0], x[1], x[2]))
        result.extend (uc.reservedBlock)

        # add the block size at the start
        lenResult = len (result) + 2
        result = Struct ("<H").pack (lenResult) + result

        return result

    def exportTableOfUseCases (self, seqRegAddresses):
        """Export all the use cases.

        The seqRegAddresses should be a dictionary using the use cases' guids as keys, and the
        location of the corresponding SequentialRegisterBlock (if any) as the value. By default
        the empty dictionary will result in and SequentialRegisterBlock's address being assumed
        to be zero,this would create an invalid Zwetschge file, but is useful while calculating
        sizes for the layout.
        """
        result = bytearray ()
        for uc in self._deviceData.useCases:
            result.extend (self.exportUseCase (uc, seqRegAddresses.get (uc.guid, 0)))
        crc = Struct ("<I").pack (crc32 (result))
        result = bytes ("Elena", 'ascii') + crc + result
        return result

    def lenTableOfUseCases (self):
        """Returns the number of bytes that exportTableOfUseCases would return"""
        seqRegAddresses = {}
        return len (self.exportTableOfUseCases(seqRegAddresses))

    def exportTableOfRegisterMaps (self):
        result = bytearray ()
        result.extend (pack24 (1)) # version
        result.extend (pack24p24s16a (0, 0, 0)) # firmware page 1
        result.extend (pack24p24s16a (0, 0, 0)) # firmware page 2
        timedRegisterLists = [
            self._deviceData.torm.init,
            self._deviceData.torm.fwPage1,
            self._deviceData.torm.fwPage2,
            self._deviceData.torm.fwStart,
            self._deviceData.torm.start,
            self._deviceData.torm.stop,
            ]
        for x in timedRegisterLists:
            result.extend (Struct ("<H").pack (len (x)))
        for x in timedRegisterLists:
            for y in x:
                result.extend (packTimedRegListEntry (y))
        crc = Struct ("<I").pack (crc32 (result))
        result = bytes ("eLENA", 'ascii') + crc + result
        return result

    def lenTableOfRegisterMaps (self):
        """Returns the number of bytes that exportTableOfRegisterMaps would return"""
        return len (self.exportTableOfRegisterMaps())


    def exportTableOfContents (self, layout):
        result = bytearray ()
        result.extend (pack24 (0x147)) # version
        result.extend (pack24p24s (layout.moduleSuffixStart, layout.moduleSuffixLen)) # Embedded-system specific data / USB specific data
        productIssuer = bytes (self._deviceData.productIssuer, 'ascii')
        if len (productIssuer) != 4:
            raise InvalidValue ("productIssuer isn't 4 bytes")
        result.extend (productIssuer)
        result.extend (self._deviceData.productCode.bytes)
        result.extend (Struct ("<I").pack (self._deviceData.systemFrequency))
        result.extend (pack24p24s (layout.torm, self.lenTableOfRegisterMaps()))
        result.extend (pack24p24s (layout.calibrationStart, layout.calibrationLen))
        result.extend (Struct ("<I").pack (layout.calibrationCrc))
        result.extend (Struct ("B").pack (len (self._deviceData.useCases)))
        result.extend (pack24p24s (layout.touc, self.lenTableOfUseCases()))

        moduleSerial = bytes (self._moduleSerial, 'ascii')
        if len (moduleSerial) != 19:
            raise InvalidValue ("moduleSerial isn't 19 bytes")
        result.extend (moduleSerial)

        crc = Struct ("<I").pack (crc32 (result))
        result = bytes ("ZWETSCHGE", 'ascii') + crc + result
        return result

    def lenTableOfContents (self):
        """Returns the number of bytes that exportTableOfContents would return"""
        # Constructing the full table just to find out the length means the ToC gets constructed
        # twice, but as a build step this inefficiency is a small cost for simpler code.  The same
        # applies to the ToUC and ToRM's similar length functions.
        layout = Layout ()
        return len (self.exportTableOfContents (layout))

    def calculateLayout (self):
        layout = Layout ()
        layout.toc = 0x2000 # the size of the reserved pages
        layout.torm = layout.toc + self.lenTableOfContents()
        layout.touc = layout.torm + self.lenTableOfRegisterMaps()
        accumulator = layout.touc + self.lenTableOfUseCases()
        for uc in self._deviceData.useCases:
            if uc.seqRegBlock is not None:
                accumulator = roundUpToPage (accumulator)
                layout.useCaseSeqRegBlocks[uc.guid] = accumulator
                accumulator = accumulator + self.lenSeqRegBlock (uc.seqRegBlock)
        
        if self._calibration is not None:
            layout.calibrationCrc = crc32 (self._calibration)
            layout.calibrationLen = len (self._calibration)
            layout.calibrationStart = accumulator
            accumulator = accumulator + layout.calibrationLen
        
        if self._moduleSuffix is not None:
            layout.moduleSuffixLen = len (self._moduleSuffix)
            layout.moduleSuffixStart = accumulator
            
        return layout

    def pack (self):
        """Export the data (returns the binary representation)"""
        layout = self.calculateLayout ()

        # Start with the reserved pages. Because we're creating a file rather than a flash image,
        # include a string at the start.
        result = bytearray ("This is a Zwetschge file\0", 'ascii')
        result.extend (bytes (layout.toc - len (result)))

        result.extend (self.exportTableOfContents (layout))
        result.extend (self.exportTableOfRegisterMaps())
        result.extend (self.exportTableOfUseCases(layout.useCaseSeqRegBlocks))

        for uc in self._deviceData.useCases:
            if uc.seqRegBlock is not None:
                start = layout.useCaseSeqRegBlocks[uc.guid]
                if len (result) > start:
                    raise RuntimeError ("Error in the ZwetschgeWriter's layout calculation")
                if len (result) < start:
                    result.extend (bytes (start - len (result)))
                result.extend (self.exportSeqRegBlock (uc.seqRegBlock))

        if self._calibration is not None:
            result.extend (self._calibration)
        if self._moduleSuffix is not None:
            result.extend (self._moduleSuffix)

        return result

class ZwetschgeWriterWithoutReserved (ZwetschgeWriter):
    """Returns a Zwetschge image without the 0x2000 byte reserved area at the start.

    All addresses in the data will assume that the 0x2000 bytes will be present when the image is
    accessed, for example the file may be used as input for a tool which writes it to flash starting
    at offset 0x2000.
    """
    def __init__ (self, deviceData, calibration, moduleSerial, moduleSuffix):
        super().__init__(deviceData, calibration, moduleSerial, moduleSuffix)

    def pack (self):
        result = super().pack()
        return result[0x2000:]
