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

#include <usecase/RawFrameSetId.hpp>

#include <vector>

namespace royale
{
    namespace usecase
    {
        class FrameGroup
        {
        public:
            bool operator== (const FrameGroup &) const;
            bool operator!= (const FrameGroup &) const;

            /**
            *  Add a frame set to this frame group.
            *
            *  @param id Index of the frame set to add.
            */
            void addFrameSet (const RawFrameSetId &id);

            /**
             * Identifiers of the RFSs in this group.
             *
             * Note: although this is named "Id" and not "Idx", the UseCaseDefinition class expects
             * these to be indices in to the UCD's array of raw frame sets, and this is enforced by
             * UCD::verifyClassInvariants().
             */
            std::vector<RawFrameSetId> m_frameSetIds;
        };
    }
}
