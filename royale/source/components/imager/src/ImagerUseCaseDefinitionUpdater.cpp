/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerUseCaseDefinitionUpdater.hpp>
#include <common/exceptions/InvalidValue.hpp>

using namespace royale::imager;


ImagerUseCaseDefinitionUpdater::ImagerUseCaseDefinitionUpdater (const ImagerUseCaseDefinition &iucd)
    : ImagerUseCaseDefinition (iucd)
{
}

//! Set the target raw frame rates for all raw frames [Hz]
void ImagerUseCaseDefinitionUpdater::setTargetFrameRate (uint16_t targetFrameRate)
{
    m_targetRate = targetFrameRate;
}

//! Set exposure times for all raw frames
void ImagerUseCaseDefinitionUpdater::setExposureTimes (const std::vector<uint32_t> &exposureTimes)
{
    auto expIt = exposureTimes.begin();
    auto expEnd = exposureTimes.end();

    for (auto &rfs : m_rawFrames)
    {
        if (expIt == expEnd)
        {
            throw royale::common::InvalidValue ("not enough exposure times");
        }

        rfs.exposureTime = *expIt;
        if (rfs.isEndOfLinkedRawFrames)
        {
            expIt++;
        }
    }
    if (expIt != expEnd)
    {
        throw royale::common::InvalidValue ("too many exposure times");
    }
}
