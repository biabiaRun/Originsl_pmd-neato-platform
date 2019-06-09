/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gmock/gmock.h>
#include <imager/ImagerSimpleHexSerialNumber.hpp>

using namespace royale::common;
using namespace royale::imager;

TEST (TestImagerSimpleHexSerialNumber, ValidConstruction)
{
    const auto regs = std::vector<uint16_t> {0xA001, 0xB002, 0xC003, 0xD004};
    const auto serial = ImagerSimpleHexSerialNumber {regs};
    ASSERT_EQ (serial.toString(), "A001-B002-C003-D004");
}

TEST (TestImagerSimpleHexSerialNumber, TooFewValues)
{
    const auto regs = std::vector<uint16_t> {0xA001, 0xB002, 0xC003};
    ASSERT_ANY_THROW (ImagerSimpleHexSerialNumber {regs});
}

TEST (TestImagerSimpleHexSerialNumber, TooManyValues)
{
    const auto regs = std::vector<uint16_t> {0xA001, 0xB002, 0xC003, 0xD004, 0xE005};
    ASSERT_ANY_THROW (ImagerSimpleHexSerialNumber {regs});
}
