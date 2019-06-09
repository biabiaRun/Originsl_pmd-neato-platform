/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/IImagerModeStrategy.hpp>
#include <imager/ISequenceGenerator.hpp>

#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/IntegerMath.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace royale::common;
using namespace royale::imager;

IImagerModeStrategy::~IImagerModeStrategy() = default;

const std::map < uint16_t, uint16_t > IImagerModeStrategy::reconfigTranslationMixedMode (
    const size_t mbCount,
    const uint16_t mbStart,
    const uint16_t mbOffset,
    const uint16_t seqIdxOffset,
    const uint16_t flags,
    const uint16_t maxParams,
    const uint16_t srParamStart,
    const uint16_t srDecodeStart,
    const std::map < uint16_t, uint16_t > &regChanges)
{
    std::map <uint16_t, uint16_t> sourceRegisters = regChanges;
    std::map <uint16_t, uint16_t> transformedRegisters;
    const uint16_t TOTAL_SEQNUM = static_cast<uint16_t> (divideRoundUp (mbOffset, seqIdxOffset));
    std::vector<uint16_t> paramList;
    std::vector<uint16_t> decodeList;
    uint16_t pParam = srParamStart;
    uint16_t pDecode = srDecodeStart;

    //this nested loops are used traverse the shadow-config address space for measurement blocks
    for (size_t mbIdx = 0; mbIdx < mbCount; mbIdx++)
    {
        const uint16_t curMBStart = static_cast<uint16_t> (mbStart + (mbIdx * mbOffset));

        for (uint16_t seqIdx = 0u; seqIdx < TOTAL_SEQNUM; seqIdx = static_cast<uint16_t> (seqIdx + 1u))
        {
            const uint16_t oneHotSeqIdx = static_cast<uint16_t> (1 << seqIdx);

            for (uint16_t regIdx = 0u; regIdx < seqIdxOffset; regIdx = static_cast<uint16_t> (regIdx + 1u))
            {
                const uint16_t seqNumBitslice = static_cast<uint16_t> (0x3F << 4);
                const uint16_t decode = static_cast<uint16_t> (mbIdx + (oneHotSeqIdx << 4) + (regIdx << 12));
                const uint16_t oldParamAdr = static_cast<uint16_t> (curMBStart + (seqIdx * seqIdxOffset) + regIdx);

                if (oldParamAdr >= static_cast<uint16_t> (curMBStart + mbOffset))
                {
                    //for the M2452 B1x AIO firmware the control section of the MB has one less entry compared to
                    //the standard sequence entries, so the general for loop is not valid for this special case
                    continue;
                }

                if (regChanges.count (oldParamAdr))
                {
                    //find if n-hot coding is possible (param must be equal and decode must have same MB and reg-index)
                    auto itExistingDecode = std::find_if (decodeList.begin(), decodeList.end(), [&] (const uint16_t &existingDecode)
                    {
                        return //decode must have same MB and reg-index
                            (existingDecode & static_cast<uint16_t> (~seqNumBitslice)) ==
                            (decode & static_cast<uint16_t> (~seqNumBitslice)) &&
                            //param must be equal
                            paramList.at (std::addressof (existingDecode) - std::addressof (decodeList[0])) ==
                            regChanges.at (oldParamAdr);
                    });

                    if (itExistingDecode != decodeList.end())
                    {
                        *itExistingDecode |= decode;
                    }
                    else
                    {
                        paramList.push_back (regChanges.at (oldParamAdr));
                        decodeList.push_back (decode);
                    }

                    sourceRegisters.erase (oldParamAdr);
                }
            }
        }
    }

    if (sourceRegisters.size())
    {
        throw RuntimeError ("reconfiguration missed some registers to translate");
    }

    if (paramList.size() > maxParams)
    {
        throw NotImplemented ("reconfiguration failed, partial reconfiguration is not implemented");
    }

    for (size_t i = 0; i < paramList.size() && i < decodeList.size(); i++)
    {
        transformedRegisters[pParam] = paramList.at (i);
        transformedRegisters[pDecode] = decodeList.at (i);
        pParam = static_cast<uint16_t> (pParam + 2u);
        pDecode = static_cast<uint16_t> (pDecode + 2u);
    }

    //this register triggers the firmware to copy the configuration, it is part of the translated
    //registers because it contains the number of parameters used, but its placement at the highest
    //register address will ensure that all other configuration is written before the trigger is set
    transformedRegisters[flags] = static_cast<uint16_t> (3u + (paramList.size() << 8));

    return transformedRegisters;
}