#!/usr/bin/python3

from zwetschge_tool.zwetschge.data_types import TableOfRegisterMaps

init = [
    (0x9401, 0x0002, 0),
    (0xA00C, 0x0135, 0),
    (0xA03E, 0x0000, 0),
    (0xA03D, 0x03C0, 0),
    (0xA008, 0x1513, 0),
    (0xA03B, 0x000A, 0),
    (0xA03A, 0xAAAB, 0),
    (0xA039, 0x1AA1, 0),
    (0xA03F, 0x0017, 0),
    (0xA001, 0x0007, 0),
    (0xA03C, 0x0000, 0),
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
