/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseDefinition.hpp>
#include <stdint.h>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/NarrowCast.hpp>

#include <algorithm>
#include <limits>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;

UseCaseDefinition::UseCaseDefinition() :
    m_identifier {},
    m_targetRate (1),
    m_imageColumns (0),
    m_imageRows (0),
    m_enableSSC (false),
    m_maxRate (1),
    m_minRate (1)
{
}

UseCaseDefinition::UseCaseDefinition (uint16_t maxFrameRate) :
    m_identifier {},
    m_targetRate (maxFrameRate),
    m_imageColumns (0),
    m_imageRows (0),
    m_enableSSC (false),
    m_maxRate (maxFrameRate),
    m_minRate (1)
{
}

bool UseCaseDefinition::operator== (const UseCaseDefinition &rhs) const
{
    if (m_typeName == rhs.m_typeName &&
            m_identifier == rhs.m_identifier &&
            m_targetRate == rhs.m_targetRate &&
            m_enableSSC == rhs.m_enableSSC &&
            m_imageColumns == rhs.m_imageColumns &&
            m_rawFrameSet.size () == rhs.m_rawFrameSet.size () &&
            m_streams.size() == rhs.m_streams.size() &&
            m_exposureGroups.size() == rhs.m_exposureGroups.size() &&
            m_minRate == rhs.m_minRate &&
            m_maxRate == rhs.m_maxRate)
    {
        for (size_t i = 0; i < m_rawFrameSet.size(); ++i)
        {
            if (m_rawFrameSet.at (i) != rhs.m_rawFrameSet.at (i))
            {
                return false;
            }
        }
        for (size_t i = 0; i < m_streams.size(); ++i)
        {
            if (* (m_streams.at (i)) != * (rhs.m_streams.at (i)))
            {
                return false;
            }
        }
        for (size_t i = 0; i < m_exposureGroups.size(); ++i)
        {
            if (m_exposureGroups.at (i) != rhs.m_exposureGroups.at (i))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool UseCaseDefinition::operator!= (const UseCaseDefinition &rhs) const
{
    return !operator== (rhs);
}

void UseCaseDefinition::verifyClassInvariants() const
{
    if (m_typeName.empty())
    {
        throw LogicError ("Unnamed Use Case");
    }
    if ( (m_targetRate > m_maxRate) || (m_minRate > m_maxRate))
    {
        throw LogicError ("Exceeds max rate");
    }
    if (m_rawFrameSet.size() == 0)
    {
        throw LogicError ("Empty use case (no frames)");
    }
    if (getRawFrameCount() >= 2047u)
    {
        // Assuming 12-bit counters, IPseudoDataInterpreter::isGreaterFrame() may consider the first
        // frame to have a greater frame number than the last frame
        throw LogicError ("Huge use case (too many frames for 12-bit counters)");
    }
    if (m_streams.size() == 0)
    {
        throw LogicError ("Empty use case (no streams)");
    }
    if (m_exposureGroups.size() == 0)
    {
        throw LogicError ("Use case has no exposure settings");
    }

    const auto numberOfExposureGroups = m_exposureGroups.size();
    auto expoGroupsUsed = std::vector<bool> (numberOfExposureGroups, false);
    for (const auto &rfs : m_rawFrameSet)
    {
        if (rfs.exposureGroupIdx >= numberOfExposureGroups)
        {
            throw LogicError ("A RawFrameSet has an out-of-bounds ExposureGroup index");
        }
        expoGroupsUsed.at (rfs.exposureGroupIdx) = true;
    }
    auto isUnused = [] (bool x)
    {
        return !x;
    };
    if (std::any_of (expoGroupsUsed.cbegin(), expoGroupsUsed.cend(), isUnused))
    {
        throw LogicError ("Unused exposure group");
    }

    for (const auto &group : m_exposureGroups)
    {
        if (group.m_name.empty())
        {
            throw LogicError ("Unnamed exposure group");
        }
        if (group.m_exposureLimits.first > group.m_exposureLimits.second)
        {
            throw LogicError ("Exposure limits are reversed");
        }
        if (group.m_exposureLimits.first > group.m_exposureTime || group.m_exposureTime > group.m_exposureLimits.second)
        {
            throw LogicError ("Exposure time is outside the exposure limits");
        }
    }

    for (const auto &stream : m_streams)
    {
        if (stream->m_id == 0)
        {
            throw LogicError ("Invalid stream id");
        }
        const auto &frameGroups = stream->m_frameGroups;
        if (frameGroups.size() == 0)
        {
            throw LogicError ("Empty stream (no frame groups)");
        }
        const auto &groupZero = frameGroups[0].m_frameSetIds;
        const auto groupSize = groupZero.size();
        if (groupSize == 0)
        {
            throw LogicError ("Empty FrameGroup (no frame sets)");
        }
        const auto numberOfRawFrameSets = m_rawFrameSet.size();
        for (const auto &rfsId : groupZero)
        {
            if (rfsId >= numberOfRawFrameSets)
            {
                throw LogicError ("A FrameGroup has an out-of-bounds RawFrameSet index");
            }
        }
        for (size_t i = 1; i < frameGroups.size(); i++)
        {
            const auto &group = frameGroups[i].m_frameSetIds;
            if (groupSize != group.size())
            {
                throw LogicError ("Different frame groups in a stream");
            }
            for (size_t j = 0; j < groupSize; j++)
            {
                if (m_rawFrameSet.at (groupZero.at (j)) != m_rawFrameSet.at (group.at (j)))
                {
                    throw LogicError ("Mismatched frames in groups in the stream");
                }
            }
        }
    }
}

void UseCaseDefinition::setTargetRate (uint16_t rate)
{
    m_targetRate = rate;
}

bool UseCaseDefinition::getSSCEnabled() const
{
    return m_enableSSC;
}

void UseCaseDefinition::setDutyCycle (double dutyCycle, const RawFrameSet *rawFrameSet)
{
    RawFrameSet::DutyCycle newDutyCycle;

    switch (narrow_cast<uint16_t> (dutyCycle * 100.0))
    {
        case 0u:
            newDutyCycle = RawFrameSet::DutyCycle::DC_0;
            break;
        case 3750u:
            newDutyCycle = RawFrameSet::DutyCycle::DC_37_5;
            break;
        case 2500u:
            newDutyCycle = RawFrameSet::DutyCycle::DC_25;
            break;
        case 5000u:
            newDutyCycle = RawFrameSet::DutyCycle::DC_50;
            break;
        case 7500u:
            newDutyCycle = RawFrameSet::DutyCycle::DC_75;
            break;
        case 10000u:
            newDutyCycle = RawFrameSet::DutyCycle::DC_100;
            break;
        default:
            throw OutOfBounds();
            break;
    }

    if (rawFrameSet == nullptr)
    {
        // for non RawFrameSet not defined set exposure time for a modulation RawFrameSets
        for (size_t idx = 0; idx < m_rawFrameSet.size(); idx++)
        {
            m_rawFrameSet.at (idx).dutyCycle = newDutyCycle;
        }
    }
    else
    {
        // otherwise set exposure time for the specified RawFrameSet
        for (size_t idx = 0; idx < m_rawFrameSet.size(); idx++)
        {
            if (&m_rawFrameSet.at (idx) == rawFrameSet)
            {
                m_rawFrameSet.at (idx).dutyCycle = newDutyCycle;
            }
        }
    }
}

void UseCaseDefinition::setExposureTime (uint32_t exposureTime, StreamId s)
{
    for (const auto setIdx : getRawFrameSetIndices (s, 0u))
    {
        if (m_rawFrameSet.at (setIdx).isModulated())
        {
            auto &group = m_exposureGroups.at (m_rawFrameSet.at (setIdx).exposureGroupIdx);
            group.m_exposureTime = exposureTime;
        }
    }
}

void UseCaseDefinition::setExposureTimes (const royale::Vector<uint32_t> &exposureTimes)
{
    const size_t nTimes = exposureTimes.size();
    if (nTimes != m_exposureGroups.size())
    {
        throw InvalidValue ("Vector of exposure times and number of exposure groups mismatch");
    }

    for (size_t idx = 0; idx < nTimes; idx++)
    {
        m_exposureGroups.at (idx).m_exposureTime = exposureTimes[idx];
    }
}

royale::Vector<uint32_t> UseCaseDefinition::getExposureTimes() const
{
    royale::Vector<uint32_t> exposureTimes;
    for (const auto &expGroup : m_exposureGroups)
    {
        exposureTimes.push_back (expGroup.m_exposureTime);
    }
    return exposureTimes;
}

uint32_t UseCaseDefinition::getExposureTimeForRawFrameSet (const RawFrameSet &set) const
{
    const auto &group = m_exposureGroups.at (set.exposureGroupIdx);
    return group.m_exposureTime;
}

const royale::Pair<uint32_t, uint32_t> UseCaseDefinition::getExposureLimits (StreamId s) const
{
    const auto setIndices = getRawFrameSetIndices (s, 0u);
    if (setIndices.empty())
    {
        throw InvalidValue ("getExposureLimits(): no exposure groups on stream!");
    }

    auto limits = royale::Pair<uint32_t, uint32_t> {std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max() };
    for (const auto setIdx : setIndices)
    {
        if (m_rawFrameSet.at (setIdx).isModulated())
        {
            const auto &group = m_exposureGroups.at (m_rawFrameSet.at (setIdx).exposureGroupIdx);
            limits.first = std::max (limits.first, group.m_exposureLimits.first);
            limits.second = std::min (limits.second, group.m_exposureLimits.second);
        }
    }

    if (limits.first > limits.second)
    {
        throw InvalidValue ("getExposureLimits(): invalid exposure limits on stream!");
    }

    return limits;
}

const royale::Vector<royale::Pair<uint32_t, uint32_t>> UseCaseDefinition::getExposureLimits() const
{
    royale::Vector<royale::Pair<uint32_t, uint32_t>> ret;

    for (const auto &group : getExposureGroups())
    {
        ret.emplace_back (group.m_exposureLimits);
    }

    return ret;
}

const royale::Vector<RawFrameSet> &UseCaseDefinition::getRawFrameSets() const
{
    return m_rawFrameSet;
}

royale::Vector<StreamId> UseCaseDefinition::getStreamIds() const
{
    royale::Vector<StreamId> streamIds;
    for (const auto &stream : m_streams)
    {
        streamIds.push_back (stream->m_id);
    }
    return streamIds;
}

royale::Vector<std::size_t> UseCaseDefinition::getRawFrameSetIndices (StreamId s, std::size_t groupIdx) const
{
    royale::Vector<std::size_t> sets;
    auto stream = getStream (s);

    const auto &frameGroup = stream->m_frameGroups.at (groupIdx);
    for (auto id : frameGroup.m_frameSetIds)
    {
        sets.push_back (id);
    }
    return sets;
}

royale::Vector<std::size_t> UseCaseDefinition::getExposureIndicesForStream (StreamId s) const
{
    royale::Vector<std::size_t> groupIndices;
    for (const auto setIdx : getRawFrameSetIndices (s, 0u))
    {
        groupIndices.push_back (m_rawFrameSet.at (setIdx).exposureGroupIdx);
    }
    return groupIndices;
}

royale::Vector<uint16_t> UseCaseDefinition::getSequenceIndicesForRawFrameSet (std::size_t set) const
{
    if (set > m_rawFrameSet.size())
    {
        throw royale::common::OutOfBounds();
    }

    size_t count = 0;
    for (std::size_t i = 0; i < set; i++)
    {
        count += m_rawFrameSet[i].countRawFrames();
    }

    royale::Vector<uint16_t> result;
    for (std::size_t i = 0; i < m_rawFrameSet[set].countRawFrames(); i++)
    {
        result.push_back (static_cast<uint16_t> (count + i));
    }
    return result;
}

std::size_t UseCaseDefinition::getRawFrameCount() const
{
    size_t count = 0;
    for (const auto &rawFrameSet : m_rawFrameSet)
    {
        count += rawFrameSet.countRawFrames();
    }
    return count;
}

//!< Number of frame groups in the given stream
std::size_t UseCaseDefinition::getFrameGroupCount (StreamId s) const
{
    const auto &stream = getStream (s);
    return stream->m_frameGroups.size();
}

uint16_t UseCaseDefinition::getTargetRate() const
{
    return m_targetRate;
}

uint16_t UseCaseDefinition::getMaxRate() const
{
    return m_maxRate;
}

uint16_t UseCaseDefinition::getMinRate() const
{
    return m_minRate;
}

void UseCaseDefinition::getImage (uint16_t &columns, uint16_t &rows) const
{
    columns = m_imageColumns;
    rows = m_imageRows;
}

void UseCaseDefinition::setImage (uint16_t columns, uint16_t rows)
{
    m_imageRows = rows;
    m_imageColumns = columns;
}

const royale::String &UseCaseDefinition::getTypeName() const
{
    return m_typeName;
}

UseCaseIdentifier UseCaseDefinition::getIdentifier() const
{
    return m_identifier;
}

const royale::Vector<ExposureGroup> &UseCaseDefinition::getExposureGroups() const
{
    return m_exposureGroups;
}

std::shared_ptr<Stream> UseCaseDefinition::createStream (StreamId id)
{
    if (id == 0)
    {
        if (m_streams.empty())
        {
            id = DEFAULT_STREAM_ID;
        }
        else
        {
            StreamId highest = 0;
            for (const auto &s : m_streams)
            {
                highest = std::max (highest, s->m_id);
            }

            if (highest == std::numeric_limits<StreamId>::max())
            {
                throw RuntimeError ("Can't allocate a StreamId");
            }
            id = static_cast<StreamId> (highest + 1);
        }
    }

    // Check if stream already exists...
    for (const auto &s : m_streams)
    {
        if (s->m_id == id)
        {
            throw InvalidValue ("Duplicate StreamId");
        }
    }

    auto stream = std::make_shared<Stream>();
    stream->m_id = id;
    m_streams.push_back (stream);
    return stream;
}

const std::shared_ptr<Stream> UseCaseDefinition::getStream (StreamId id) const
{
    // Check if stream already exists...
    for (const auto &s : m_streams)
    {
        if (s->m_id == id)
        {
            return s;
        }
    }
    throw InvalidValue ("Unknown StreamId");
}

ExposureGroupIdx UseCaseDefinition::createExposureGroup (const royale::String &name, royale::Pair<uint32_t, uint32_t> limits, uint32_t exposure)
{
    // exposure group names must be unique.
    for (const auto &e : m_exposureGroups)
    {
        if (e.m_name == name)
        {
            throw InvalidValue ("Duplicate exposure group");
        }
    }

    m_exposureGroups.emplace_back (ExposureGroup{ name, limits, exposure });

    return static_cast<ExposureGroupIdx> (m_exposureGroups.size() - 1);
}

RawFrameSet UseCaseDefinition::createGrayRFS (ExposureGroupIdx exposureGroup, ExposureGray expoOn, uint32_t modulationFrequency)
{
    RawFrameSet rfs
    {
        modulationFrequency,
        RawFrameSet::PhaseDefinition::GRAYSCALE,
        expoOn == ExposureGray::On ? RawFrameSet::DutyCycle::DC_AUTO : RawFrameSet::DutyCycle::DC_0,
        exposureGroup
    };
    return rfs;
}

void UseCaseDefinition::constructNonMixedUseCase (royale::Vector<RawFrameSet> rawFrameSets)
{
    if (!m_streams.empty())
    {
        throw LogicError ("Calling constructNonMixedUseCase after creating a stream");
    }

    if (!m_rawFrameSet.empty())
    {
        throw LogicError ("Calling constructNonMixedUseCase after adding sets directly to m_rawFrameSet");
    }

    auto stream = createStream (DEFAULT_STREAM_ID);
    constructFrameGroup (stream, std::move (rawFrameSets),  RawFrameSet::Alignment::CLOCK_ALIGNED);
}

void UseCaseDefinition::constructFrameGroup (std::shared_ptr<Stream> stream, royale::Vector<RawFrameSet> groupSets, RawFrameSet::Alignment alignment, bool appendPrevious)
{
    using Alignment = RawFrameSet::Alignment;

    const auto firstIdx = m_rawFrameSet.size();
    if (firstIdx == 0 && alignment != Alignment::CLOCK_ALIGNED)
    {
        throw LogicError ("The first set of the Use Case must be CLOCK_ALIGNED");
    }

    FrameGroup *frameGroup;
    if (appendPrevious)
    {
        frameGroup = &stream->m_frameGroups.back();
    }
    else
    {
        frameGroup = stream->createFrameGroup();
    }

    const auto nFrameSets = groupSets.size();
    for (std::size_t i = 0 ; i < nFrameSets ; i++)
    {
        // The calling mechanism took a copy not a reference of the groupSets (unless the caller
        // std::moved them), so they can be std::moved here.
        auto rfs = std::move (groupSets[i]);

        // Alignment
        if (i == 0 && alignment == Alignment::CLOCK_ALIGNED)
        {
            rfs.alignment = Alignment::CLOCK_ALIGNED;
        }
        else if (alignment == Alignment::STOP_ALIGNED)
        {
            rfs.alignment = Alignment::STOP_ALIGNED;
        }
        else if (alignment == Alignment::NEXTSTOP_ALIGNED)
        {
            rfs.alignment = Alignment::NEXTSTOP_ALIGNED;
        }
        else
        {
            rfs.alignment = Alignment::START_ALIGNED;
        }

        m_rawFrameSet.push_back (std::move (rfs));
        frameGroup->addFrameSet (firstIdx + i);
    }
}
