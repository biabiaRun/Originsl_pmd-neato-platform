#!/usr/bin/python3

from zwetschge_tool.zwetschge.data_types import TableOfRegisterMaps

init = [
   (0x9401, 0x0002, 0),
   (0x8423, 0x00A2, 0),
   (0xA03A, 0x5555, 0),
   (0xA03E, 0x0000, 0),
   (0xA03F, 0x000F, 0),
   (0xA008, 0x1513, 0),
   (0xA03B, 0x0005, 0),
   (0xA039, 0x16A1, 0),
   (0xA001, 0x0007, 0),
   (0xA03C, 0x0000, 0),
   (0xA03D, 0x04D0, 0),
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
