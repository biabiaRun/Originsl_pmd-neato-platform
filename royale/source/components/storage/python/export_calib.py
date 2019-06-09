#!/usr/bin/python3

import argparse
import os
from struct import Struct
from zlib import crc32

TOC_OFFSET_VERSION = 13

def unpack24 (inArray, offset):
    return inArray[offset] + (inArray[offset+1] << 8) + (inArray[offset+2] << 16) 

def unpack24p24s (inArray, offset):
    p = unpack24(inArray, offset)
    s = unpack24(inArray, offset+3)
    return p, s
    
def main (infile, outfile = None):

    if isinstance(infile, str):
        if os.path.isfile(infile):
            inFile = open (infile, "rb")
            zwetschgeFile = bytearray(inFile.read())
    elif isinstance(infile, bytearray):
        zwetschgeFile = infile
    else:
        raise FileNotFoundError('Not supported file format found!')

    if outfile is not None:
            outFile = open (outfile, "wb")

    if zwetschgeFile[:9] != bytes ("ZWETSCHGE", 'ascii'):
        zwetschgeFile = zwetschgeFile[0x2000:]    

    if zwetschgeFile[:9] != bytes ("ZWETSCHGE", 'ascii'):
        raise FileNotFoundError("Not a valid Zwetschge file")
        quit()
    
    zwetschgeVersion = unpack24(zwetschgeFile, TOC_OFFSET_VERSION)
    #print ("Zwetschge version : " + hex(zwetschgeVersion))
    
    calib = None
    if zwetschgeVersion == 0x147:
        p, s = unpack24p24s(zwetschgeFile, 52)

        #print("Calib size : " + str(s))
        
        p = p - 0x2000

        calib = zwetschgeFile[p:p+s]

        # verify calibration crc
        if crc32(calib) != Struct('<I').unpack(zwetschgeFile[58:62])[0]:
            raise Exception('Calibration checksum mismatch!')

        if outfile is not None:
            outFile.write(calib)
    else:
        raise NotImplementedError('Wrong Zwetschge version!')
        
    return calib
    
if (__name__ == "__main__"):
    parser = argparse.ArgumentParser (usage = __doc__)
    parser.add_argument ("--infile", help="input file")
    parser.add_argument ("--outfile", help="output file")
    options = parser.parse_args()

    if not options.outfile:
        options.outfile = None
        print('No output file will be generated!')

    if not options.infile:
        raise FileNotFoundError('No infile parameter given!')

    calib = main(options.infile, options.outfile)
