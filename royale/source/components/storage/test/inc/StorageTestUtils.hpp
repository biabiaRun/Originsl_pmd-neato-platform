/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace royale
{
    namespace test
    {
        namespace utils
        {
            /**
             * Allows test code to be written as `stringlikeMagicNumber ("MAGIC")` instead of
             * <tt>std::vector<uint8_t> ({'M', 'A', 'G', 'I', 'C'})</tt>.
             *
             * If the padToLength is non zero, then the magic number will be padded with zeros,
             * so `stringlikeMagicNumber ("MAGIC", 8)` instead of
             * <tt>std::vector<uint8_t> ({'M', 'A', 'G', 'I', 'C', 0, 0, 0})</tt>.
             */
            inline std::vector<uint8_t> stringlikeMagicNumber (const std::string &src, std::size_t padToLength = 0)
            {
                std::vector<uint8_t> dest;
                for (const auto c : src)
                {
                    dest.emplace_back (static_cast<uint8_t> (c));
                }
                if (padToLength)
                {
                    assert (src.size() <= padToLength);
                    for (auto i = src.size(); i < padToLength ; ++i)
                    {
                        dest.push_back (0);
                    }
                }
                return dest;
            }
        }
    }
}
