#!/usr/bin/python3

# Copyright (C) 2018 Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""Utility to calculate the same 16-byte IDs as royale::common::UuidlikeIdentifier."""

import argparse
import doctest
import struct
import sys
import uuid
import zlib

def hashUuidlikeIdentifier (s):
    """Given a string, calculate the same Uuid-like ID as royale::common::UuidlikeIdentifier's
    UuidlikeIdentifier(royale::String) constructor. This includes a hash to handle strings of
    any length.
    
    The object returned supports the same operations as a Python uuid.

    Examples (also unit-tests, when run with doctest):

    >>> hashUuidlikeIdentifier ("a")
    UUID('61000000-0000-0000-0000-000043beb7e8')

    >>> hashUuidlikeIdentifier ("example 5 phase")
    UUID('6578616d-706c-6520-3520-70686485d74a')

    >>> hashUuidlikeIdentifier ("example 9 phase")
    UUID('6578616d-706c-6520-3920-706810ef778d')

    >>> hashUuidlikeIdentifier ("This doesn't look like a GUID")
    UUID('54686973-2064-6f65-736e-2774212a3da4')

    >>> hashUuidlikeIdentifier ("This string doesn't look like a GUID, and is longer than a GUID is expected to be")
    UUID('54686973-2073-7472-696e-67209920adb6')
    """
    s = bytearray (s, "ascii")
    if len (s) < 12:
        result = s.ljust (12, b'\0')
    else:
        result = s [0:12]
    crc = struct.Struct ("<I").pack (zlib.crc32 (s))
    result = bytes (result + crc)
    return uuid.UUID (bytes=result)

def castUuidlikeIdentifier (s):
    """Given a string of exactly 16 ASCII characters, calculate the same Uuid-like ID as
    royale::common::UuidlikeIdentifier's UuidlikeIdentifier(UuidlikeIdentifier::datatype)
    constructor.  This does not do any hash or CRC, instead it creates IDs matching the result of
    C++ code that uses the datatype constructor:
    
        static const royale::processing::ProcessingParameterId CommonId2Frequencies
        {
            royale::processing::ProcessingParameterId::datatype{{'c', 'o', 'm', 'm', 'o', 'n', ' ', 'i', 'd', ' ', '2', ' ', 'f', 'r', 'e', 'q'}}
        };

    The object returned supports the same operations as a Python uuid.

    Examples (also unit-tests, when run with doctest):

    >>> castUuidlikeIdentifier ("LowNoiseExtended")
    UUID('4c6f774e-6f69-7365-4578-74656e646564')
    """
    result = bytes (s, "ascii")
    if len (result) is not 16:
        raise ValueError ("Only strings of exactly 16 characters are supported")
    return uuid.UUID (bytes=result)

def main():
    if sys.version_info[0] == 2:
        print ("This tool does not support Python 2.")
        print ("Backporting would require changes to the bytearray, crc32 and uuid method calls.")
        # in other words, you could reuse the docstrings
        exit (-1)

    parser = argparse.ArgumentParser()
    parser.add_argument ("sources", metavar="sources", nargs='*', type=str, help="strings to create hashes of")
    options = parser.parse_args()

    # run the unit tests (will be silent unless they fail)
    doctest.testmod()

    for s in options.sources:
        print ("%s : {%s}" % (s, hashUuidlikeIdentifier (s)))

if __name__ == "__main__":
    main ()
