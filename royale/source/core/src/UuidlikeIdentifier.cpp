/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <common/Crc32.hpp>
#include <common/EndianConversion.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/RoyaleLogger.hpp>
#include <common/UuidlikeIdentifier.hpp>

#include <algorithm>
#include <sstream>
#include <vector>

using namespace royale::common;

std::array<uint8_t, 16> royale::common::parseRfc4122AsUuid (const royale::String &s)
{
    using datatype = std::array<uint8_t, 16>;
    datatype data{};
    char openBrace = 0, dash1 = 0, dash2 = 0, dash3 = 0, dash4 = 0, closeBrace = 0;
    uint32_t time_low = 0;
    uint16_t time_mid = 0;
    uint16_t time_high_and_version = 0;
    uint16_t clock_seq_and_reserved_and_low = 0;
    uint64_t node = 0; // 6 octets used, 2 highest bytes will be zero

    std::istringstream iss (s.toStdString());
    iss >> std::hex >> openBrace >> time_low >> dash1 >> time_mid >> dash2 >> time_high_and_version >> dash3 >> clock_seq_and_reserved_and_low >> dash4 >> node >> closeBrace;
    if (iss.fail() || openBrace != '{' || dash1 != '-' || dash2 != '-' || dash3 != '-' || dash4 != '-' || closeBrace != '}')
    {
        throw InvalidValue ("Can't parse as a UUID");
    }
    std::vector<uint8_t> result;
    pushBackBe32 (result, time_low);
    pushBackBe16 (result, time_mid);
    pushBackBe16 (result, time_high_and_version);
    pushBackBe16 (result, clock_seq_and_reserved_and_low);
    pushBackBe48 (result, node);
    std::copy_n (result.begin(), 16, data.begin());

    return data;
}

std::array<uint8_t, 16> royale::common::hashUuidlikeIdentifier (const royale::String &s)
{
    // Copy the bytes of the string in to the opaque data. There might be multiple long strings in
    // use that start with the same 16 characters, so put a hash of the whole string in to the last
    // 4 bytes (using the hashing function that's already in royalecore).
    using datatype = std::array<uint8_t, 16>;
    datatype data{};

    const auto maxVerbatimBytes = std::tuple_size<datatype>::value - 4;
    static_assert (maxVerbatimBytes == 12, "incorrect assumptions in UuidlikeIdentifier");
    std::copy_n (
        reinterpret_cast<const uint8_t *> (s.data()),
        std::min (s.size(), maxVerbatimBytes),
        data.begin());
    hostToBuffer32 (&data[maxVerbatimBytes], calculateCRC32 (reinterpret_cast<const uint8_t *> (s.data()), s.size()));

    return data;
}
