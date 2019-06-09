/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <usecase/ExposureGroup.hpp>
#include <usecase/FrameGroup.hpp>
#include <royale/StreamId.hpp>
#include <royale/Vector.hpp>

namespace royale
{
    namespace usecase
    {
        /**
         * General stream ID for the first stream in any Use Case, however users should always use
         * getStreamIds() instead of hardcoding this.  This is provided only for defining the Use
         * Cases with standard code.
         *
         * This is guaranteed to be non-zero, and (for any reasonable number of streams), creating
         * additional IDs with static_cast<uint16_t> (DEFAULT_STREAM_ID + i) won't wrap round.
         */
        const StreamId DEFAULT_STREAM_ID
        {
            0xdefa
        };

        class Stream
        {
        public:
            bool operator== (const Stream &) const;
            bool operator!= (const Stream &) const;

            /**
            *  Create a new frame group and add it to m_frameGroups.
            *  Note the pointer returned should be regarded as non-owning reference;
            *  it may be invalidated by destruction of the Stream or by another call
            *  to createFrameGroup().
            *
            *  @return the frame group.
            */
            FrameGroup *createFrameGroup();

            StreamId                         m_id;
            royale::Vector<FrameGroup>       m_frameGroups;
        };
    }
}
