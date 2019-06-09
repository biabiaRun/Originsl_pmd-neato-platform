/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <collector/BufferActionCalcIndividual.hpp>
#include <common/NarrowCast.hpp>

#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <set>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;

BufferActionCalcIndividual::BufferActionCalcIndividual() = default;
BufferActionCalcIndividual::~BufferActionCalcIndividual() = default;

IBufferActionCalc::Result BufferActionCalcIndividual::calculateActions (
    const royale::usecase::UseCaseDefinition &useCase,
    const std::vector<CollectorFrameGroup> &frameGroupList,
    const royale::Vector<std::size_t> & /* ignored for TransmissionMode::INDIVIDUAL */) const
{
    // The maximum number of buffers needed to hold a complete frame group from a stream (a stream
    // with duplicated frames can need varying numbers of buffers per frame group).
    std::map<StreamId, std::size_t> bufferCountsForStream;
    for (const auto streamId : useCase.getStreamIds())
    {
        bufferCountsForStream[streamId] = 0u;
    }

    // Calculate the BufferActionMap from the FrameGroupList.
    BufferActionMap bam;

    // The frames are received as useCaseFrameCount buffers, each containing one frame.
    auto useCaseFrameCount = useCase.getRawFrameCount();
    for (uint16_t i = 0; i < useCaseFrameCount; i++)
    {
        // as this is a map, the next line can create entries
        BufferAction &action = bam[i];
        // these buffers are individual frames
        action.mapping.resize (1);
    }

    for (std::size_t groupIdx = 0; groupIdx < frameGroupList.size(); groupIdx++)
    {
        // Work which buffers will contain the frames for this group
        const auto &group = frameGroupList[groupIdx];
        std::set<uint16_t> buffersForGroup;
        for (uint16_t i = 0; i < group.sequence.size(); i++)
        {
            // The logic is written in this way for easy comparison to the BufferActionCalcSuper
            const auto sequenceId = group.sequence[i];
            const auto bufferId = sequenceId; // there's only one frame in the buffer
            const auto frameInBuffer = 0u; // sequenceId - bufferId;
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
        // For individual frames, the number of buffers is simply the number of frames in any
        // frame group of the stream, and multiply by 2 for double-buffering.
        bufferCount += 2 * bufferCountsForStream.at (streamId);
    }

    // Get height of each frame's image, in pixels.  The width is already in m_imageWidth, but the
    // height doesn't need to be a member variable.
    uint16_t imageWidth;
    uint16_t imageHeight;
    useCase.getImage (imageWidth, imageHeight);
    // add pseudodata
    imageHeight++;

    return {bam, imageWidth, imageHeight, bufferCount};
}
