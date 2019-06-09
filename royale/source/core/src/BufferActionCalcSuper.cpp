/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <collector/BufferActionCalcSuper.hpp>
#include <common/NarrowCast.hpp>

#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <set>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;

BufferActionCalcSuper::BufferActionCalcSuper() = default;
BufferActionCalcSuper::~BufferActionCalcSuper() = default;

IBufferActionCalc::Result BufferActionCalcSuper::calculateActions (
    const royale::usecase::UseCaseDefinition &useCase,
    const std::vector<CollectorFrameGroup> &frameGroupList,
    const royale::Vector<std::size_t> &blockSizes) const
{
    // A map from the individual sequenceIds to the relevant bufferId (where bufferId means the
    // sequenceId of the first frame in the buffer).
    std::vector<uint16_t> seqToBuffer;

    // The maximum number of buffers needed to hold a complete frame group from a stream (a stream
    // with duplicated frames can need varying numbers of buffers per frame group).
    std::map<StreamId, std::size_t> bufferCountsForStream;
    for (const auto streamId : useCase.getStreamIds())
    {
        bufferCountsForStream[streamId] = 0u;
    }

    // Calculate the BufferActionMap from the buffer sizes and FrameGroupList.
    BufferActionMap bam;
    uint16_t bufferId = 0;
    for (const auto size : blockSizes)
    {
        // as this is a map, the next line can create entries
        BufferAction &action = bam[bufferId];
        action.mapping.resize (size);
        for (std::size_t i = 0; i < size ; i++)
        {
            seqToBuffer.emplace_back (bufferId);
        }
        bufferId = narrow_cast<uint16_t> (bufferId + size);
    }

    for (std::size_t groupIdx = 0; groupIdx < frameGroupList.size(); groupIdx++)
    {
        // Work which buffers will contain the frames for this group
        const auto &group = frameGroupList[groupIdx];
        std::set<uint16_t> buffersForGroup;
        for (uint16_t i = 0; i < group.sequence.size(); i++)
        {
            const auto sequenceId = group.sequence[i];
            const auto bufferId = seqToBuffer.at (sequenceId);
            const auto frameInBuffer = sequenceId - bufferId;
            BufferAction &action = bam.at (bufferId);
            action.mapping.at (frameInBuffer).push_back (MapFramesTo {groupIdx, i});
            buffersForGroup.emplace (bufferId);
        }

        // Work out when the group will be ready
        const auto readyWhen = *std::max_element (buffersForGroup.cbegin(), buffersForGroup.cend());
        BufferAction &action = bam.at (readyWhen);
        action.ready.emplace_back (groupIdx);
        auto &bufferCount = bufferCountsForStream.at (group.streamId);
        bufferCount = std::max (bufferCount, buffersForGroup.size());
    }

    // Calculate the number of buffers needed for double-buffering.
    // Assume there are no duplicate frames that are shared between different streams.
    std::size_t bufferCount = 0u;
    for (const auto streamId : useCase.getStreamIds())
    {
        // Double-buffering a stream with only one frame group requires enough buffers to hold
        // each pair of (frameGroup from first cycle, frameGroup from next cycle).
        //
        // Double-buffering a stream with multiple frame groups can, assuming there are no frame
        // drops, get a smaller number by calculating adject groups.  However, assuming that there
        // may be frame drops we need at least enough buffers to handle the pair (any frameGroup
        // from one cycle, any frameGroup from the next cycle), and possibly more.
        bufferCount += 2 * bufferCountsForStream.at (streamId);
    }

    // Get height of each frame's image, in pixels.  The width is already in m_imageWidth, but the
    // height doesn't need to be a member variable.
    uint16_t imageWidth;
    uint16_t imageHeight;
    useCase.getImage (imageWidth, imageHeight);
    // add pseudodata
    imageHeight++;
    const auto biggestBuffer = *std::max_element (blockSizes.cbegin(), blockSizes.cend());

    return {bam, imageWidth, narrow_cast<uint16_t> (imageHeight * biggestBuffer), bufferCount};
}
