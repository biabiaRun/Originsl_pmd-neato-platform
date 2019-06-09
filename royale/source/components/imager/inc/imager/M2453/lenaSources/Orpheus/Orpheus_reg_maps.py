#!/usr/bin/python3

from zwetschge_tool.zwetschge.data_types import TableOfRegisterMaps

init = [
    (0xa001, 0x0007, 0),
    (0xa008, 0x1513, 0),
    (0xa00c, 0x0135, 0),
    (0x9401, 0x0002, 0),
    (0xa039, 0x1aa1, 0),
    (0xa03a, 0xaaab, 0),
    (0xa03b, 0x000a, 0),
    (0xa03c, 0x0000, 0),
    (0xa03d, 0x03c0, 0),
    (0xa03e, 0x0000, 0),
    (0xa03f, 0x0017, 0),   
    (0xa087, 0x8003, 0),
]

fwPage1 = []

fwPage2 = []

fwStart = [
    (0xA02A, 0x0001, 0),
]

start = [
    (0x9400, 0x0001, 0),
]

stop = [
    (0x9400, 0x0000, 500000),
]

torm = TableOfRegisterMaps (
    init,
    fwPage1,
    fwPage2,
    fwStart,
    start,
    stop
    )
