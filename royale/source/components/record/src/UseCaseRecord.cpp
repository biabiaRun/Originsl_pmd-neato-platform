/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <record/UseCaseRecord.hpp>
#include <NarrowCast.hpp>
#include <stdint.h>
#include <common/exceptions/RuntimeError.hpp>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using namespace royale::record;

//! generate custom definition for recordings
UseCaseRecord::UseCaseRecord (const RecordUseCaseDefinition &definition)
{
    m_typeName = definition.frameHeader.useCaseName;
    m_targetRate = definition.frameHeader.targetFrameRate;

    m_imageColumns = definition.frameHeader.numColumns;
    m_imageRows = definition.frameHeader.numRows;

    std::vector<FrameGroup> tmpFrameGroups (definition.frameGroups.size());
    for (auto i = 0u; i < definition.frameGroups.size(); ++i)
    {
        auto &curFrameGroup = definition.frameGroups[i];
        tmpFrameGroups[i].m_frameSetIds.resize (curFrameGroup.numRawFrameSets);
        for (auto j = 0u; j < curFrameGroup.numRawFrameSets; ++j)
        {
            tmpFrameGroups[i].m_frameSetIds[j] = static_cast<RawFrameSetId> (curFrameGroup.rawFrameSetIdxs[j]);
        }
    }

    for (auto &curStream : definition.streams)
    {
        auto stream = createStream (curStream.streamId);
        stream->m_frameGroups.resize (curStream.numFrameGroups);
        for (auto i = 0u; i < curStream.numFrameGroups; ++i)
        {
            stream->m_frameGroups[i] = tmpFrameGroups[curStream.frameGroupIdxs[i]];
        }
    }

    for (auto &curRawFrameSet : definition.rawFrameSets)
    {
        RawFrameSet set;
        set.modulationFrequency = curRawFrameSet.modFreq;
        set.phaseDefinition = static_cast<RawFrameSet::PhaseDefinition> (curRawFrameSet.phaseDefinition);
        set.dutyCycle = static_cast<RawFrameSet::DutyCycle> (curRawFrameSet.dutyCycle);
        set.exposureGroupIdx = curRawFrameSet.exposureGroupIdx;
        set.alignment = static_cast<RawFrameSet::Alignment> (curRawFrameSet.alignment);
        set.tEyeSafety = curRawFrameSet.eyeSafetyGap;

        m_rawFrameSet.push_back (set);
    }

    for (auto &curExpoGroup : definition.exposureGroups)
    {
        ExposureGroup expoGroup;
        expoGroup.m_name = curExpoGroup.exposureGroupName;
        expoGroup.m_exposureTime = curExpoGroup.exposureTime;
        expoGroup.m_exposureLimits.first = curExpoGroup.exposureMin;
        expoGroup.m_exposureLimits.second = curExpoGroup.exposureMax;

        m_exposureGroups.push_back (expoGroup);
    }
}
